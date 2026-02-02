#include "engine.hpp"

#include "types.hpp"


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

  if(moveList.empty()) {
    return -INF + (mDepth - depth);
  }

  int bestScore = -INF;

  for(const auto& move : moveList) {
    pos.doMove(move);

    int score = -negaMax(pos, depth -1, alpha, beta, stoken);
    pos.undoMove(move);

    if(score > bestScore) {
      if(depth == mCurrentDepth) {
      mLastBestMove = move; 
      }
    }

    if(bestScore > alpha) {
      alpha = bestScore;
    }

    if(alpha >= beta) {
      break;
    }
  }

  return bestScore;
}

Move Engine::search(Position& pos, std::stop_token stoken) {
    mLastBestMove = Move();
    mCurrentDepth = 1;
    while (!stoken.stop_requested() && mCurrentDepth <= mDepth) {
        //search for best move
        //go through all moves
        //evaluate position
        //go to next depth and analyse best move for opponent and so on
        int score = negaMax(pos, mCurrentDepth, -INF, INF, stoken);


        if(stoken.stop_requested()) {
          break;
        }

        mCurrentEval = score;
        mCurrentDepth++;
    }
    return mLastBestMove;
}

int Engine::evaluate(Position& pos) {
  int score = 0;

  return (pos.mSideToMove == WHITE) ? score : -score;
}

void Engine::setDepth(int depth) {
    mDepth = depth;
}

int Engine::getDepth() {
    return mDepth;
}

Engine::Engine() {
    ZobristHashing zobrist;
}

Engine::~Engine() {
}

ZobristHashing::ZobristHashing() {

}

ZobristHashing::~ZobristHashing() {
    
}
