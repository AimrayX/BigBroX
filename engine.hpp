#pragma once

#include "position.hpp"

#include <cstdint>
#include <string>
#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <stop_token>
#include <atomic>


class ZobristHashing {
private:
    std::unordered_map<int, std::vector<uint64_t>> pieceHashes;
    uint64_t currentHash;

public:
    ZobristHashing(int numPieces);

    void movePiece(int piece, int from, int to);

    uint64_t getHash() const;

    ZobristHashing();
    ~ZobristHashing();
};


class Engine
{
private:
    int mDepth;
    std::chrono::time_point<std::chrono::steady_clock> mStartTime;
    int mTimeAllocated;
    bool mStop;
    long long nodes;
    static constexpr int PST[6][64] = {
    {
      0,0,0,0,0,0,0,0,
      0,0,-5,-10,-10,-5,0,0,
      5,0,5,10,10,5,0,5,
      0,0,10,20,20,10,0,0,
      0,0,10,20,20,10,0,0,
      0,0,5,10,10,5,0,0,
      5,5,5,10,10,5,5,5,
      20,20,20,20,20,20,20,20,
    },
    {
      -50,-10,-10,-10,-10,-10,-10,-50,
      -10,-5,0,5,5,0,-5,-10,
      -5,0,10,10,10,10,0,-5,
      -5,0,10,20,20,10,0,-5,
      -5,0,10,25,25,10,0,-5,
      -10,0,5,10,10,5,0,-10,
      -10,-5,0,0,0,0,-5,-10,
      -50,-10,-10,-10,-10,-10,-10,-50,
    },
    {
      -10,-10,-10,-10,-10,-10,-10,-10,
       10,10,0,0,0,0,10,10,
       5,20,0,10,10,0,20,5,
       0,0,20,20,20,20,0,0,
       0,0,10,10,10,10,0,0,
       -5,0,5,10,10,5,0,-5,
      -10,-5,-5,-5,-5,-5,-5,-10,
      -10,-10,-10,-10,-10,-10,-10,-10,
    },
    {
       -10,10,10,20,20,10,10,-10,
       0,5,5,10,10,5,5,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       10,10,10,10,10,10,10,10,
       0,0,0,0,0,0,0,0,
    },
    {
      -10,0,0,-5,0,0,0,-10,
       0,0,0,10,5,0,0,0,
       5,10,0,5,0,0,10,5,
       0,0,5,5,5,5,0,0,
       0,0,0,5,5,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
    },
    {
       20,10,0,-5,-5,0,10,20,
       10,0,-5,-5,-5,-5,0,10,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
    }
    };

static constexpr int mvv_lva[6][6] = {
//Attacker: P, N, B, R, Q, K
    {105, 205, 305, 405, 505, 605}, // Victim: Pawn
    {104, 204, 304, 404, 504, 604}, // Victim: Knight
    {103, 203, 303, 403, 503, 603}, // Victim: Bishop
    {102, 202, 302, 402, 502, 602}, // Victim: Rook
    {101, 201, 301, 401, 501, 601}, // Victim: Queen
    {100, 200, 300, 400, 500, 600}  // Victim: King (Shouldn't happen, but safe to add)
};

public:
    std::atomic<int> mCurrentDepth;
    std::atomic<int> mCurrentEval;
    std::atomic<unsigned long long> mTimeSpentMs;

    Move mLastBestMove;

    static const int MAX_PLY = 64;
    Move pvTable[MAX_PLY][MAX_PLY]; 
    int pvLength[MAX_PLY];

    void setDepth(int depth);
    int getDepth();

    int scoreMove(const Move& m, Position& pos);
    uint64_t mAlgebraicToBit(std::string alge);
    Move search(Position& pos, int timeLimitMs, std::stop_token stoken);
    int evaluate(Position& pos);
    int quiescence(Position& pos, int alpha, int beta, std::stop_token& stoken);
    int negaMax(Position& pos, int depth, int alpha, int beta, std::stop_token& stoken);
    void pickMove(MoveList& list, int moveNum);

    ZobristHashing zobristHashing;

    Engine();
    ~Engine();
};

