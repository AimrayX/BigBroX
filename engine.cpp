#include "engine.hpp"

#include "types.hpp"
#include <cstdlib>


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

int Engine::negaMax(Position& pos, int depth, int alpha, int beta, std::stop_token& stoken) {
  if (stoken.stop_requested()) return 0;

  if(depth == 0) {
    return Engine::evaluate(pos);
  }

  std::vector<Move> moveList;
  pos.getMoves(pos.mSideToMove, moveList);
  int bestScore = -INF;
  int movesSearched = 0;

  for(const auto& move : moveList) {
    std::cout << "\nbefore Move" << std::endl;
    pos.printBoard();

    pos.doMove(move);

    std::cout << "\nafter Move" << std:: endl;
    pos.printBoard();

    Color sideJustMoved = (pos.mSideToMove == WHITE) ? BLACK : WHITE;
    int kingSquare = __builtin_ctzll(pos.pieces[sideJustMoved][KING]);

    if (pos.isSquareAttacked(kingSquare, pos.mSideToMove)) {
        pos.undoMove(move);
        continue;
    }

    movesSearched++;

    int score = -negaMax(pos, depth -1, -alpha, -beta, stoken);
    pos.undoMove(move);

    if(score > bestScore) {
      if(depth == mCurrentDepth) {
      mLastBestMove = move; 
      }
      bestScore = score;
    }

    if(bestScore > alpha) {
      alpha = bestScore;
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

Move Engine::search(Position& pos, std::stop_token stoken) {
    mLastBestMove = Move();
    mCurrentDepth = 1;
    while (!stoken.stop_requested() && mCurrentDepth <= mDepth) {
        int score = negaMax(pos, mCurrentDepth, -INF, INF, stoken);


        if(stoken.stop_requested()) {
          break;
        }

        mCurrentEval = score;
        mCurrentDepth++;
        std::cout << "info depth " << std::to_string(mCurrentDepth) << std::endl;
        std::cout << "info score cp " << std::to_string(mCurrentEval) << std::endl;
        std::cout << "info time " << std::to_string(mTimeSpentMs) << std::endl;
    }
    std::cout << "search end" << std::endl;
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
  mDepth = 1;
  mTimeSpentMs = 0;
}

Engine::~Engine() {
}

ZobristHashing::ZobristHashing() {

}

ZobristHashing::~ZobristHashing() {
    
}
