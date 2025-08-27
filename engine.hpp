#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <string>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <random>

class Engine
{
private:
    

    



public:
    int mDepth;

    std::string mLastBestMove;

    uint64_t mAlgebraicToBit(std::string alge);
    std::string mSearch();

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