#pragma once

#include <stdint.h>

#include <atomic>
#include <cstdint>
#include <stop_token>
#include <string>

#include "position.hpp"
#include "transposition.hpp"

class Engine {
 private:
  int mDepth;
  std::chrono::time_point<std::chrono::steady_clock> mStartTime;
  int mTimeAllocated;
  bool mStop;
  long long nodes;
  Move killerMoves[64][2];
  int historyMoves[2][64][64];
  std::vector<Move> getPV(Position& pos, int depth);

  static constexpr int mvv_lva[6][6] = {
      // Attacker: P, N, B, R, Q, K
      {105, 205, 305, 405, 505, 605},  // Victim: Pawn
      {104, 204, 304, 404, 504, 604},  // Victim: Knight
      {103, 203, 303, 403, 503, 603},  // Victim: Bishop
      {102, 202, 302, 402, 502, 602},  // Victim: Rook
      {101, 201, 301, 401, 501, 601},  // Victim: Queen
      {100, 200, 300, 400, 500, 600}   // Victim: King (Shouldn't happen, but safe to add)
  };

 public:
  std::atomic<int> mCurrentDepth;
  std::atomic<int> mCurrentEval;
  std::atomic<unsigned long long> mTimeSpentMs;

  Move mLastBestMove;

  static const int MAX_PLY = 64;
  Move pvTable[MAX_PLY][MAX_PLY];
  int pvLength[MAX_PLY];
  TranspositionTable tt;

  void setDepth(int depth);
  int getDepth();

  int scoreMove(const Move& m, Position& pos, int ply);
  uint64_t mAlgebraicToBit(std::string alge);
  Move search(Position& pos, int timeLimitMs, std::stop_token stoken);
  int evaluate(Position& pos);
  int quiescence(Position& pos, int alpha, int beta, std::stop_token& stoken);
  int negaMax(Position& pos, int depth, int alpha, int beta, std::stop_token& stoken);
  void pickMove(MoveList& list, int moveNum);

  Engine();
  ~Engine();
};
