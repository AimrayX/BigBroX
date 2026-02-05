#include "engine.hpp"

#include <chrono>
#include <iostream>

#include "attack.hpp"
#include "types.hpp"
#include "utils.hpp"

const int INF = 1000000;

// Add to top of engine.cpp
const uint64_t FILE_A = 0x0101010101010101ULL;
const uint64_t FILE_H = 0x8080808080808080ULL;

// Precompute masks for passed pawn checks
uint64_t PASSED_MASK[2][64];  // [Color][Square]
uint64_t ISOLATED_MASK[64];   // [File]

void initEvalMasks() {
  for (int sq = 0; sq < 64; sq++) {
    int file = sq % 8;
    int rank = sq / 8;

    // ISOLATED MASK: The files to the left and right
    ISOLATED_MASK[file] = 0ULL;
    if (file > 0) ISOLATED_MASK[file] |= (FILE_A << (file - 1));
    if (file < 7) ISOLATED_MASK[file] |= (FILE_A << (file + 1));

    // PASSED MASK: All squares in front of the pawn on adjacent files + own file
    // WHITE
    PASSED_MASK[WHITE][sq] = 0ULL;
    uint64_t forward = 0ULL;
    for (int r = rank + 1; r < 8; r++) forward |= (1ULL << (r * 8 + file));

    PASSED_MASK[WHITE][sq] |= forward;
    if (file > 0) PASSED_MASK[WHITE][sq] |= (forward >> 1);
    if (file < 7) PASSED_MASK[WHITE][sq] |= (forward << 1);

    // BLACK (Mirror)
    PASSED_MASK[BLACK][sq] = 0ULL;
    forward = 0ULL;
    for (int r = rank - 1; r >= 0; r--) forward |= (1ULL << (r * 8 + file));

    PASSED_MASK[BLACK][sq] |= forward;
    if (file > 0) PASSED_MASK[BLACK][sq] |= (forward >> 1);
    if (file < 7) PASSED_MASK[BLACK][sq] |= (forward << 1);
  }
}

// Tuning values (in centipawns)
const int DOUBLED_PENALTY = 15;
const int ISOLATED_PENALTY = 20;
const int PASSED_BONUS[8] = {0, 10, 30, 50, 75, 100, 150, 200};  // Bonus increases by rank

int evalPawns(const Position& pos, Color side) {
  int score = 0;
  uint64_t myPawns = pos.pieces[side][PAWN];
  uint64_t enemyPawns = pos.pieces[side ^ 1][PAWN];

  uint64_t tempPawns = myPawns;
  while (tempPawns) {
    int sq = __builtin_ctzll(tempPawns);
    int file = sq % 8;
    int rank = sq / 8;
    int relativeRank = (side == WHITE) ? rank : (7 - rank);

    // 1. PASSED PAWN (No enemy pawns in front or adjacent files)
    if (!(PASSED_MASK[side][sq] & enemyPawns)) {
      score += PASSED_BONUS[relativeRank];
    }

    // 2. ISOLATED PAWN (No friendly pawns on adjacent files)
    if (!(ISOLATED_MASK[file] & myPawns)) {
      score -= ISOLATED_PENALTY;
    }

    // 3. DOUBLED PAWN (Another friendly pawn on the same file)
    // We mask the file and check if popcount > 1
    if (__builtin_popcountll(myPawns & (FILE_A << file)) > 1) {
      // We apply penalty once per pawn, effectively penalizing the pair
      score -= DOUBLED_PENALTY;
    }

    tempPawns &= (tempPawns - 1);
  }
  return score;
}

// King Safety Penalties (in centipawns)
const int KING_SHIELD_PENALTY = 20;  // Per missing shield pawn

const uint64_t KING_SIDE_MASK_W = (1ULL << 5) | (1ULL << 6) | (1ULL << 7);      // f1, g1, h1
const uint64_t KING_SIDE_MASK_B = (1ULL << 61) | (1ULL << 62) | (1ULL << 63);   // f8, g8, h8
const uint64_t QUEEN_SIDE_MASK_W = (1ULL << 0) | (1ULL << 1) | (1ULL << 2);     // a1, b1, c1
const uint64_t QUEEN_SIDE_MASK_B = (1ULL << 56) | (1ULL << 57) | (1ULL << 58);  // a8, b8, c8

int evalKingSafety(const Position& pos, Color side) {
  // 1. Fast Exit: If King is not on back rank, skip safety check
  // (This saves time in endgames/middle-of-board chaos)
  uint64_t kingBB = pos.pieces[side][KING];
  if (side == WHITE) {
    if (!(kingBB & 0xFFULL)) return 0;  // King not on rank 1
  } else {
    if (!(kingBB & 0xFF00000000000000ULL)) return 0;  // King not on rank 8
  }

  int score = 0;
  int kingSq = __builtin_ctzll(kingBB);
  int file = kingSq % 8;
  uint64_t pawns = pos.pieces[side][PAWN];

  // 2. Bitwise Shield Check
  if (file > 4) {  // Kingside
    uint64_t shieldMask = (side == WHITE) ? (KING_SIDE_MASK_W << 8) : (KING_SIDE_MASK_B >> 8);

    // We want pawns to be present in the shieldMask
    // XOR gives us the bits where pawns are MISSING
    uint64_t missing = (shieldMask & ~pawns);

    // Count missing pawns instantly
    int missingCount = __builtin_popcountll(missing);
    score -= (missingCount * KING_SHIELD_PENALTY);

  } else if (file < 3) {  // Queenside
    uint64_t shieldMask = (side == WHITE) ? (QUEEN_SIDE_MASK_W << 8) : (QUEEN_SIDE_MASK_B >> 8);
    uint64_t missing = (shieldMask & ~pawns);
    int missingCount = __builtin_popcountll(missing);
    score -= (missingCount * KING_SHIELD_PENALTY);
  }

  // (Optional: You can keep the Open File penalty logic here if you want,
  // but usually the shield check covers 90% of cases).

  return score;
}

const int ROOK_OPEN_FILE_BONUS = 10;      // Lowered (Mobility covers the rest)
const int ROOK_SEMI_OPEN_FILE_BONUS = 5;  // Lowered
const int ROOK_ON_7TH_BONUS = 20;         // Keep this high

const int BISHOP_PAIR_BONUS = 30;

// New Mobility Weights (Weight per attacked square)
const int KNIGHT_MOBILITY = 4;
const int BISHOP_MOBILITY = 5;
const int ROOK_MOBILITY = 3;
// const int QUEEN_MOBILITY = 2;

int evalPieces(const Position& pos, Color side) {
  int score = 0;
  uint64_t occupancy = pos.occupancies[2];
  uint64_t myPawns = pos.pieces[side][PAWN];
  uint64_t enemyPawns = pos.pieces[side ^ 1][PAWN];

  // --- BISHOPS ---
  uint64_t bishops = pos.pieces[side][BISHOP];
  if (__builtin_popcountll(bishops) >= 2) score += BISHOP_PAIR_BONUS;

  while (bishops) {
    int sq = __builtin_ctzll(bishops);

    // Mobility: Count attacked squares
    uint64_t attacks = attack::getBishopAttacks(sq, occupancy);
    score += __builtin_popcountll(attacks) * BISHOP_MOBILITY;

    bishops &= (bishops - 1);
  }

  // --- KNIGHTS ---
  uint64_t knights = pos.pieces[side][KNIGHT];
  while (knights) {
    int sq = __builtin_ctzll(knights);
    // We use the precomputed attack table directly
    uint64_t attacks = attack::knightAttacks[sq];
    // We don't care about occupancy for knight mobility (freedom of movement)
    // possibly mask with ~my_pieces to count only valid moves
    score += __builtin_popcountll(attacks & ~pos.occupancies[side]) * KNIGHT_MOBILITY;
    knights &= (knights - 1);
  }

  // --- ROOKS ---
  uint64_t rooks = pos.pieces[side][ROOK];
  while (rooks) {
    int sq = __builtin_ctzll(rooks);
    int file = sq % 8;
    int rank = sq / 8;

    // Mobility
    uint64_t attacks = attack::getRookAttacks(sq, occupancy);
    score += __builtin_popcountll(attacks) * ROOK_MOBILITY;

    // Static Open File Bonus (Reduced, but still useful)
    uint64_t fileMask = FILE_A << file;
    if (!(myPawns & fileMask)) {
      if (!(enemyPawns & fileMask)) {
        score += ROOK_OPEN_FILE_BONUS;
      } else {
        score += ROOK_SEMI_OPEN_FILE_BONUS;
      }
    }

    // Rook on 7th
    int relativeRank = (side == WHITE) ? rank : (7 - rank);
    if (relativeRank == 6) {
      int enemyKingRank = __builtin_ctzll(pos.pieces[side ^ 1][KING]) / 8;
      if ((side == WHITE && enemyKingRank == 7) || (side == BLACK && enemyKingRank == 0)) {
        score += ROOK_ON_7TH_BONUS;
      }
    }
    rooks &= (rooks - 1);
  }

  // --- QUEENS ---
  // (Optional: Queen mobility is expensive/noisy, but good for "centralization")
  /*
  uint64_t queens = pos.pieces[side][QUEEN];
  while (queens) {
      int sq = __builtin_ctzll(queens);
      uint64_t attacks = attack::getQueenAttacks(sq, occupancy);
      score += __builtin_popcountll(attacks) * QUEEN_MOBILITY;
      queens &= (queens - 1);
  }
  */

  return score;
}

std::vector<Move> Engine::getPV(Position& pos, int depth) {
  std::vector<Move> pv;
  int ply = 0;
  while (ply < depth) {
    Move m = tt.probeMove(pos.getHash());
    if (m.from == 0 && m.to == 0) break;  // No move in TT
    MoveList moves;
    pos.getMoves(pos.mSideToMove, moves);
    bool legal = false;
    for (int i = 0; i < moves.count; i++) {
      if (moves.moves[i].from == m.from && moves.moves[i].to == m.to) {
        legal = true;
        break;
      }
    }
    if (!legal) break;
    pv.push_back(m);
    pos.doMove(m);
    ply++;
  }
  for (int i = pv.size() - 1; i >= 0; i--) {
    pos.undoMove(pv[i]);
  }
  return pv;
}

void Engine::pickMove(MoveList& list, int moveNum) {
  int bestIndex = -1;
  int bestScore = -1000000;

  // 1. Find the move with the highest score
  for (int i = moveNum; i < list.count; ++i) {
    if (list.moves[i].score > bestScore) {
      bestScore = list.moves[i].score;
      bestIndex = i;
    }
  }

  // 2. Swap it with the move at 'moveNum' so it gets searched next
  if (bestIndex != -1) {
    // Swap moves
    Move tempMove = list.moves[moveNum];
    list.moves[moveNum] = list.moves[bestIndex];
    list.moves[bestIndex] = tempMove;
  }
}

int Engine::scoreMove(const Move& m, Position& pos, int ply) {
  int score = 0;

  // Look up what pieces are involved using the board array
  int attacker = pos.board[m.from];
  int victim = pos.board[m.to];

  // 1. MVV-LVA for Standard Captures
  if (victim != NOPIECE) {
    score = 10000 + mvv_lva[victim][attacker];
  }
  // 2. Handle En Passant
  else if (attacker == PAWN && (1ULL << m.to) == pos.mEnPassentSquare) {
    score = 10000 + mvv_lva[PAWN][PAWN];
  }
  // 3. Killers (CRITICAL FIX HERE)
  // Check if ply is valid (>= 0) because Quiescence passes -1 to disable this
  else if (ply >= 0 && ply < MAX_PLY) {
    if (m.from == killerMoves[ply][0].from && m.to == killerMoves[ply][0].to) {
      score = 9000;
    } else if (m.from == killerMoves[ply][1].from && m.to == killerMoves[ply][1].to) {
      score = 8000;
    }
  }

  // 4. Score Promotions
  if (m.promotion != NOPIECE) {
    if (m.promotion == QUEEN)
      score += 9000;
    else if (m.promotion == KNIGHT)
      score += 4000;
    else
      score += 1000;
  }

  if (score == 0) {
    score = historyMoves[pos.mSideToMove][m.from][m.to];
  }

  return score;
}

int Engine::quiescence(Position& pos, int alpha, int beta, std::stop_token& stoken) {
  if (stoken.stop_requested()) return 0;

  if ((nodes & 2047) == 0) {
    auto now = std::chrono::steady_clock::now();
    long long elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - mStartTime).count();
    if (elapsed >= mTimeAllocated) {
      mStop = true;
      return 0;
    }
  }
  nodes++;

  // OPTIMIZATION: Stand-pat using only material/PSQT (very cheap!)
  int standPat = pos.posEval.positionScore;
  standPat = (pos.mSideToMove == WHITE) ? standPat : -standPat;
  
  // OPTIMIZATION: Delta pruning
  // If we're so far behind that even capturing a queen can't help, give up
  const int BIG_DELTA = 925;  // Queen value + margin
  if (standPat + BIG_DELTA < alpha) {
    return alpha;  // Futile position, no point searching
  }

  // Beta cutoff with stand-pat
  if (standPat >= beta) {
    return beta;
  }

  // Update alpha
  if (standPat > alpha) {
    alpha = standPat;
  }

  // Generate and search captures
  MoveList captureList;
  pos.getCaptures(pos.mSideToMove, captureList);

  // Score captures using MVV-LVA
  for (int i = 0; i < captureList.count; i++) {
    Move& move = captureList.moves[i];
    int victim = pos.board[move.to];
    int attacker = pos.board[move.from];

    if (victim != NOPIECE && attacker != NOPIECE) {
      move.score = mvv_lva[victim][attacker];
    } else if (move.promotion != NOPIECE) {
      move.score = 1000 + pieceValues[move.promotion];
    } else {
      move.score = 0;
    }
  }

  // Search captures in order of MVV-LVA
  for (int i = 0; i < captureList.count; i++) {
    pickMove(captureList, i);
    Move move = captureList.moves[i];

    // OPTIMIZATION: SEE pruning for bad captures
    // Skip captures that lose material (you'd need to implement SEE)
    // For now, skip low-scoring captures when far below alpha
    if (move.score < 100 && standPat + 200 < alpha) {
      continue;  // Probably a bad capture
    }

    pos.doMove(move);

    // Check legality
    Color sideJustMoved = (pos.mSideToMove == WHITE) ? BLACK : WHITE;
    int kingSquare = __builtin_ctzll(pos.pieces[sideJustMoved][KING]);

    if (pos.isSquareAttacked(kingSquare, pos.mSideToMove)) {
      pos.undoMove(move);
      continue;
    }

    int score = -quiescence(pos, -beta, -alpha, stoken);
    pos.undoMove(move);

    if (stoken.stop_requested()) return 0;

    if (score >= beta) return beta;
    if (score > alpha) alpha = score;
  }

  return alpha;
}

int Engine::negaMax(Position& pos, int depth, int alpha, int beta, std::stop_token& stoken) {
  if (stoken.stop_requested()) return 0;

  if ((nodes & 2047) == 0) {
    auto now = std::chrono::steady_clock::now();
    long long elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - mStartTime).count();

    if (elapsed >= mTimeAllocated) {
      mStop = true;
    }
  }
  nodes++;

  if (mStop) return 0;

  int ply = mCurrentDepth - depth;
  pvLength[ply] = ply;

  if (ply > 0) {
    if (pos.isRepetition() || pos.mHalfMove >= 100) {
      return 0;
    }
  }

  // --- TT PROBE ---
  int ttScore;
  Move ttMove = Move::null();
  // Try to retrieve a cutoff score
  if (tt.probe(pos.getHash(), depth, ply, alpha, beta, ttScore, ttMove)) {
    // If we get a valid cutoff (exact, beta, or alpha), we can return immediately!
    // Exception: If we are at the root (ply 0), we usually want to search to ensure we have a PV,
    // unless it's a mate score. For simplicity, we return here.
    if (ply > 0) return ttScore;
  }

  if (depth == 0) {
    return quiescence(pos, alpha, beta, stoken);
  }

  MoveList moveList;
  pos.getMoves(pos.mSideToMove, moveList);

  for (int i = 0; i < moveList.count; i++) {
    if (ttMove.from != 0 &&  // Check if ttMove is valid
        moveList.moves[i].from == ttMove.from && moveList.moves[i].to == ttMove.to) {
      moveList.moves[i].score = 2000000;  // Higher than any capture
      break;
    } else {
      moveList.moves[i].score = scoreMove(moveList.moves[i], pos, ply);
    }
  }

  int bestScore = -INF;
  Move bestMove = Move::null();
  int originalAlpha = alpha;
  int movesSearched = 0;

  for (int i = 0; i < moveList.count; i++) {
    pickMove(moveList, i);

    pos.doMove(moveList.moves[i]);

    Color sideJustMoved = (pos.mSideToMove == WHITE) ? BLACK : WHITE;
    int kingSquare = __builtin_ctzll(pos.pieces[sideJustMoved][KING]);

    if (pos.isSquareAttacked(kingSquare, pos.mSideToMove)) {
      pos.undoMove(moveList.moves[i]);
      continue;
    }

    movesSearched++;

    int score = -negaMax(pos, depth - 1, -beta, -alpha, stoken);
    pos.undoMove(moveList.moves[i]);

    if (stoken.stop_requested()) return 0;

    if (score > bestScore) {
      bestScore = score;
      bestMove = moveList.moves[i];
      // Always update PV for the best move found so far
      pvTable[ply][ply] = moveList.moves[i];

      // Copy the rest of the PV from the child position
      for (int j = ply + 1; j < pvLength[ply + 1]; j++) {
        pvTable[ply][j] = pvTable[ply + 1][j];
      }
      // PV length includes our move plus the child's PV
      pvLength[ply] = (pvLength[ply + 1] > ply + 1) ? pvLength[ply + 1] : (ply + 1);

      if (score > alpha) {
        alpha = score;
      }
    }

    if (alpha >= beta) {
      if (pos.board[moveList.moves[i].to] == NOPIECE) {
        int mover = (pos.mSideToMove == WHITE) ? WHITE : BLACK;
        int from = moveList.moves[i].from;
        int to = moveList.moves[i].to;

        int bonus = depth * depth;

        if (historyMoves[mover][from][to] < 5000) {
          historyMoves[mover][from][to] += bonus;
        }

        if (ply >= 0 && ply < MAX_PLY) {
          killerMoves[ply][1] = killerMoves[ply][0];
          killerMoves[ply][0] = moveList.moves[i];
        }
      }
      tt.store(pos.getHash(), depth, ply, beta, TT_BETA, moveList.moves[i]);

      return beta;
    }
  }

  if (movesSearched == 0) {
    int kingSq = __builtin_ctzll(pos.pieces[pos.mSideToMove][KING]);

    Color opponent = (pos.mSideToMove == WHITE) ? BLACK : WHITE;
    if (pos.isSquareAttacked(kingSq, opponent)) {
      return -INF + ply;
    } else {
      return 0;
    }
  }
  TTFlag flag = TT_ALPHA;
  if (bestScore > originalAlpha) {
    flag = TT_EXACT;
  }

  tt.store(pos.getHash(), depth, ply, bestScore, flag, bestMove);

  return bestScore;
}

Move Engine::search(Position& pos, int timeLimitMs, std::stop_token stoken) {
  // pos.printBoard();

  // tt.clear()
  mStartTime = std::chrono::steady_clock::now();
  mTimeAllocated = timeLimitMs;
  mStop = false;
  nodes = 0;

  mLastBestMove = Move::null();
  mCurrentEval = 0;
  mCurrentDepth = 1;

  for (int c = 0; c < 2; c++) {
    for (int f = 0; f < 64; f++) {
      for (int t = 0; t < 64; t++) {
        historyMoves[c][f][t] = 0;
      }
    }
  }

  for (int i = 0; i < MAX_PLY; i++) {
    killerMoves[i][0] = Move::null();
    killerMoves[i][1] = Move::null();
  }

  for (int depth = 1; depth <= mDepth; depth++) {
    int score = negaMax(pos, mCurrentDepth, -INF, INF, stoken);

    if (mStop || stoken.stop_requested()) {
      break;
    }

    std::vector<Move> pvLine = getPV(pos, depth);

    // Print PV
    std::cout << "info depth " << depth << " score cp " << score << " pv";

    for (const Move& m : pvLine) {
      std::cout << " " << util::moveToString(m);
    }
    std::cout << std::endl;

    // Only update if we have a valid PV
    if (!pvLine.empty()) {
      mLastBestMove = pvLine[0];
    }
    mCurrentDepth++;
  }

  // Safety check: ensure we have a valid legal move
  bool moveIsValid = false;
  if (mLastBestMove.from != 0 || mLastBestMove.to != 0) {
    // Check if this move is actually legal
    MoveList moveList;
    pos.getMoves(pos.mSideToMove, moveList);
    for (int i = 0; i < moveList.count; i++) {
      if (moveList.moves[i].from == mLastBestMove.from &&
          moveList.moves[i].to == mLastBestMove.to &&
          moveList.moves[i].promotion == mLastBestMove.promotion) {
        moveIsValid = true;
        break;
      }
    }
  }

  if (!moveIsValid) {
    // No valid move found, generate any legal move
    MoveList moveList;
    pos.getMoves(pos.mSideToMove, moveList);
    if (moveList.count > 0) {
      mLastBestMove = moveList.moves[0];
    } else {
      // No legal moves at all (checkmate or stalemate)
      std::cout << "No legal moves available!" << std::endl;
    }
  }

  // std::cout << "search end" << std::endl;
  std::cout << "bestmove " << util::moveToString(mLastBestMove) << std::endl;

  return mLastBestMove;
}

int Engine::evaluate(Position& pos) {
  // Get current state
  StateInfo& st = pos.history[pos.gamePly];

  // Material + PSQT is already incremental via pos.posEval.positionScore
  int score = pos.posEval.positionScore;

  // Check if we have cached evaluation components
  if (!st.evalCache.valid) {
    // Compute and cache all evaluation components
    st.evalCache.pawnScore[WHITE] = evalPawns(pos, WHITE);
    st.evalCache.pawnScore[BLACK] = evalPawns(pos, BLACK);
    st.evalCache.safetyScore[WHITE] = evalKingSafety(pos, WHITE);
    st.evalCache.safetyScore[BLACK] = evalKingSafety(pos, BLACK);
    st.evalCache.mobilityScore[WHITE] = evalPieces(pos, WHITE);
    st.evalCache.mobilityScore[BLACK] = evalPieces(pos, BLACK);
    st.evalCache.valid = true;
  }

  // Use cached values
  score += (st.evalCache.pawnScore[WHITE] - st.evalCache.pawnScore[BLACK]);
  score += (st.evalCache.safetyScore[WHITE] - st.evalCache.safetyScore[BLACK]);
  score += (st.evalCache.mobilityScore[WHITE] - st.evalCache.mobilityScore[BLACK]);

  return (pos.mSideToMove == WHITE) ? score : -score;
}

void Engine::setDepth(int depth) { mDepth = depth; }

int Engine::getDepth() { return mDepth; }

Engine::Engine() : tt(64) {
  mCurrentDepth = 0;
  mCurrentEval = 0;
  mDepth = 20;
  mTimeSpentMs = 0;

  // Initialize PV table
  for (int i = 0; i < MAX_PLY; i++) {
    pvLength[i] = 0;
    for (int j = 0; j < MAX_PLY; j++) {
      pvTable[i][j] = Move::null();
    }
  }
}

Engine::~Engine() {}
