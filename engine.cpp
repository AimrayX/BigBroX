#include "engine.hpp"

#include "types.hpp"
#include "utils.hpp"

#include <chrono>
#include <iostream>
#include <random>

const int INF = 1000000;

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
        if (m.from == killerMoves[ply][0].from && 
            m.to == killerMoves[ply][0].to) {
            score = 9000;
        }
        else if (m.from == killerMoves[ply][1].from && 
            m.to == killerMoves[ply][1].to) {
            score = 8000;
        }
    }

    // 4. Score Promotions
    if (m.promotion != NOPIECE) {
        if (m.promotion == QUEEN) score += 9000;
        else if (m.promotion == KNIGHT) score += 4000;
        else score += 1000;
    }

    if (score == 0) {
        score = historyMoves[pos.mSideToMove][m.from][m.to];
    }

    return score;
}

int Engine::quiescence(Position& pos, int alpha, int beta, std::stop_token& stoken) {

    if(stoken.stop_requested()) {
      return 0;
    }

    if ((nodes & 2047) == 0) {
      auto now = std::chrono::steady_clock::now();
      long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - mStartTime).count();

      if (elapsed >= mTimeAllocated) {
          mStop = true;
      }
    }
    nodes++;

    if (mStop) return 0;

    int stand_pat = evaluate(pos);

    if (stand_pat >= beta) {
        return beta;
    }

    if (stand_pat > alpha) {
        alpha = stand_pat;
    }

    MoveList moveList;
    pos.getMoves(pos.mSideToMove, moveList);

    // 1. Score the moves
    for (int i = 0; i < moveList.count; i++) {
        moveList.moves[i].score = scoreMove(moveList.moves[i], pos, -1);
    }

    // 2. Loop and Sort
    for (int i = 0; i < moveList.count; i++) {
        // SORTING: Pick the best move first!
        pickMove(moveList, i);
        Move move = moveList.moves[i];

        // --- OPTIMIZATION & FIX ---
        // Quiet moves have a score of 0. 
        // Captures and Promotions have scores > 1000.
        // Since the list is sorted, as soon as we hit a 0, we are done!
        if (move.score < 1000) {
             break; 
        }

        // Now we search ALL interesting moves (Captures, En Passant, Promotions)
        // without needing slow bitwise checks.
        pos.doMove(move);
        int score = -quiescence(pos, -beta, -alpha, stoken);
        pos.undoMove(move);

        if(stoken.stop_requested()) return 0;

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }

    return alpha;
}

int Engine::negaMax(Position& pos, int depth, int alpha, int beta, std::stop_token& stoken) {

  if (stoken.stop_requested()) return 0;

  if ((nodes & 2047) == 0) {
      auto now = std::chrono::steady_clock::now();
      long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - mStartTime).count();

      if (elapsed >= mTimeAllocated) {
          mStop = true;
      }
  }
  nodes++;

  if (mStop) return 0;

  int ply = mCurrentDepth -depth;
  pvLength[ply] = ply;

  // --- TT PROBE ---
    int ttScore;
    Move ttMove = Move();
    // Try to retrieve a cutoff score
    if (tt.probe(pos.getHash(), depth, ply, alpha, beta, ttScore, ttMove)) {
        // If we get a valid cutoff (exact, beta, or alpha), we can return immediately!
        // Exception: If we are at the root (ply 0), we usually want to search to ensure we have a PV, 
        // unless it's a mate score. For simplicity, we return here.
        if (ply > 0) return ttScore; 
    }

  if(depth == 0) {
    return quiescence(pos, alpha, beta, stoken);
  }

  MoveList moveList;
  pos.getMoves(pos.mSideToMove, moveList);

  for (int i = 0; i < moveList.count; i++) {
        if (ttMove.from != 0 && // Check if ttMove is valid
            moveList.moves[i].from == ttMove.from &&
            moveList.moves[i].to == ttMove.to) {

            moveList.moves[i].score = 2000000; // Higher than any capture
            break; 
        } else {
             moveList.moves[i].score = scoreMove(moveList.moves[i], pos, ply);
        }
  }

  int bestScore = -INF;
  Move bestMove = Move();
  int originalAlpha = alpha;
  int movesSearched = 0;

  for(int i = 0; i < moveList.count; i++) {

    pickMove(moveList, i);

    pos.doMove(moveList.moves[i]);

    Color sideJustMoved = (pos.mSideToMove == WHITE) ? BLACK : WHITE;
    int kingSquare = __builtin_ctzll(pos.pieces[sideJustMoved][KING]);

    if (pos.isSquareAttacked(kingSquare, pos.mSideToMove)) {
        pos.undoMove(moveList.moves[i]);
        continue;
    }

    movesSearched++;

    int score = -negaMax(pos, depth -1, -beta, -alpha, stoken);
    pos.undoMove(moveList.moves[i]);

    if(stoken.stop_requested()) return 0;

    if(score > bestScore) {

      bestScore = score;
      // Always update PV for the best move found so far
      pvTable[ply][ply] = moveList.moves[i];

      // Copy the rest of the PV from the child position
      for(int j = ply + 1; j < pvLength[ply + 1]; j++) {
        pvTable[ply][j] = pvTable[ply + 1][j];
      }
      // PV length includes our move plus the child's PV
      pvLength[ply] = (pvLength[ply + 1] > ply + 1) ? pvLength[ply + 1] : (ply + 1);

      if(score > alpha) {
        alpha = score;
      }
    }

    if(alpha >= beta) {
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
    //pos.printBoard();

    //tt.clear()
    mStartTime = std::chrono::steady_clock::now();
    mTimeAllocated = timeLimitMs;
    mStop = false;
    nodes = 0;

    mLastBestMove = Move();
    mCurrentEval = 0;
    mCurrentDepth = 1;
 
    for (int c = 0; c < 2; c++) {
      for (int f = 0; f < 64; f++) {
        for (int t = 0; t < 64; t++) {
            historyMoves[c][f][t] = 0;
        }
      }
    }

    for(int i=0; i<MAX_PLY; i++) {
      killerMoves[i][0] = Move();
      killerMoves[i][1] = Move();
    }

    for(int depth = 1; depth <= mDepth; depth++) {

        int score = negaMax(pos, mCurrentDepth, -INF, INF, stoken);

        if(mStop || stoken.stop_requested()) {
          break;
        }


        // Print PV
        std::cout << "info depth " << depth << " score cp " << score << " pv";
        for (int i = 0; i < pvLength[0]; i++) {
          Move m = pvTable[0][i];
          std::cout << " " << util::moveToString(m);
        }
        std::cout << std::endl;

        // Only update if we have a valid PV
        if(pvLength[0] > 0) {
            mLastBestMove = pvTable[0][0];
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
    
    //std::cout << "search end" << std::endl;
    std::cout << "bestmove " << util::moveToString(mLastBestMove) << std::endl;

    return mLastBestMove;
}

int Engine::evaluate(Position& pos) {
    static const int pieceValues[] = {
        100,   // PAWN
        300,   // KNIGHT
        320,   // BISHOP
        500,   // ROOK
        900,   // QUEEN
        20000  // KING
    };

    int score = 0;

    // 2. Sum up material and positional advantage for both sides
    for (int p = 0; p < 6; p++) {
        uint64_t bitboard = pos.pieces[WHITE][p]; 

        while (bitboard) {
            int square = __builtin_ctzll(bitboard);

            score += pieceValues[p];

            score += PST[p][square]; 

          bitboard &= (bitboard - 1);
        }
    }

    for (int p = 0; p < 6; p++) {
        uint64_t bitboard = pos.pieces[BLACK][p]; 

        while (bitboard) {
            int square = __builtin_ctzll(bitboard);

            score -= pieceValues[p];

            score -= PST[p][square ^ 56]; 

          bitboard &= (bitboard - 1);
        }
    }

    // 3. Return score from the perspective of the side to move
    // If it's White's turn, a positive score is good.
    // If it's Black's turn, we must negate the score because Negamax always maximizes "my" score.
    return (pos.mSideToMove == WHITE) ? score : -score;
}

void Engine::setDepth(int depth) {
    mDepth = depth;
}

int Engine::getDepth() {
    return mDepth;
}

Engine::Engine() : tt(64) {
  mCurrentDepth = 0;
  mCurrentEval = 0;
  mDepth = 15;
  mTimeSpentMs = 0;
  
  // Initialize PV table
  for(int i = 0; i < MAX_PLY; i++) {
    pvLength[i] = 0;
    for(int j = 0; j < MAX_PLY; j++) {
      pvTable[i][j] = Move();
    }
  }
}

Engine::~Engine() {
}

