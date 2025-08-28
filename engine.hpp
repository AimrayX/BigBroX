#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "position.hpp"

#include <string>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <random>
#include <stop_token>
#include <atomic>

class Engine
{
private:
    int mDepth;

public:
    std::atomic<int> mCurrentDepth;
    std::atomic<int> mCurrentEval;
    std::atomic<unsigned long long> mTimeSpentMs;

    std::string mLastBestMove;

    void setDepth(int depth);
    int getDepth();

    uint64_t mAlgebraicToBit(std::string alge);
    std::string search(std::stop_token stoken, Position position);

    Engine();
    ~Engine();
};

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



#endif