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

public:
    std::atomic<int> mCurrentDepth;
    std::atomic<int> mCurrentEval;
    std::atomic<unsigned long long> mTimeSpentMs;

    Move mLastBestMove;

    // PV Table Constants
    static const int MAX_PLY = 64;
    // The Table: pvTable[ply][0] is the best move at that ply
    Move pvTable[MAX_PLY][MAX_PLY]; 
    int pvLength[MAX_PLY];

    void setDepth(int depth);
    int getDepth();

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

