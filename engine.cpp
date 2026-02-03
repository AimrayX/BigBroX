#include "engine.hpp"

#include "types.hpp"
#include "utils.hpp"

#include <chrono>
#include <iostream>
#include <random>

const int INF = 1000000;

ZobristHashing::ZobristHashing(int numPieces) {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dis;

        for (int i = 0; i < numPieces; ++i) {
            pieceHashes[i] = std::vector<uint64_t>(64);
            for (int j = 0; j < 64; ++j) {
                pieceHashes[i][j] = dis(gen);
            }
        }
        currentHash = 0;
    }

void ZobristHashing::movePiece(int piece, int from, int to) {
    currentHash ^= pieceHashes[piece][from];
    currentHash ^= pieceHashes[piece][to];
}

uint64_t ZobristHashing::getHash() const {
    return currentHash;
}

void Engine::pickMove(MoveList& list, int moveNum) {
    int bestIndex = -1;
    int bestScore = -1000000; // Start with a very low score

    // 1. Find the move with the highest score
    for (int i = moveNum; i < list.count; ++i) {
        if (list.scores[i] > bestScore) {
            bestScore = list.scores[i];
            bestIndex = i;
        }
    }

    // 2. Swap it with the move at 'moveNum' so it gets searched next
    if (bestIndex != -1) {
        // Swap moves
        Move tempMove = list.moves[moveNum];
        list.moves[moveNum] = list.moves[bestIndex];
        list.moves[bestIndex] = tempMove;

        // Swap scores (to keep the parallel arrays synced)
        int tempScore = list.scores[moveNum];
        list.scores[moveNum] = list.scores[bestIndex];
        list.scores[bestIndex] = tempScore;
    }
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

    for (int i = 0; i < moveList.count; i++) {
        // --- FILTER: ONLY CAPTURES ---
        Color enemy = (pos.mSideToMove == WHITE) ? BLACK : WHITE;
        if (pos.occupancies[enemy] & (1ULL << moveList.moves[i].to)) {
            pos.doMove(moveList.moves[i]);
            // Recursively call quiescence
            int score = -quiescence(pos, -beta, -alpha, stoken);
            pos.undoMove(moveList.moves[i]);

            if(stoken.stop_requested()) {
              return 0;
            }

            if (score >= beta) {
                return beta;
            }
            if (score > alpha) {
                alpha = score;
            }
        }
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

  if(depth == 0) {
    return quiescence(pos, alpha, beta, stoken);
  }

  MoveList moveList;
  pos.getMoves(pos.mSideToMove, moveList);
  int bestScore = -INF;
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
      break;
    }
  }

  if (movesSearched == 0) {
      int kingSq = __builtin_ctzll(pos.pieces[pos.mSideToMove][KING]);

      Color opponent = (pos.mSideToMove == WHITE) ? BLACK : WHITE;
      if (pos.isSquareAttacked(kingSq, opponent)) {
          return -INF + (mDepth - depth);
      } else {
          return 0;
      }
  }

  return bestScore;
}

Move Engine::search(Position& pos, int timeLimitMs, std::stop_token stoken) {
    pos.printBoard();

    mStartTime = std::chrono::steady_clock::now();
    mTimeAllocated = timeLimitMs;
    mStop = false;
    nodes = 0;

    mLastBestMove = Move();
    mCurrentEval = 0;
    mCurrentDepth = 1;

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
  // 1. Define piece values (Standard: P=100, N=300, etc.)
    // Note: King is given a huge value to ensure the engine protects it above all else.
    static const int pieceValues[] = {
        100,   // PAWN
        300,   // KNIGHT
        320,   // BISHOP (slightly more than knight is common)
        500,   // ROOK
        900,   // QUEEN
        20000  // KING
    };

    int score = 0;

    // 2. Sum up material for both sides
    for (int p = 0; p < 6; p++) {
        // __builtin_popcountll counts the number of '1' bits (pieces) in the bitboard
        int whiteCount = __builtin_popcountll(pos.pieces[WHITE][p]);
        int blackCount = __builtin_popcountll(pos.pieces[BLACK][p]);

        score += pieceValues[p] * (whiteCount - blackCount);
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

Engine::Engine() {
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

ZobristHashing::ZobristHashing() {

}

ZobristHashing::~ZobristHashing() {
    
}
