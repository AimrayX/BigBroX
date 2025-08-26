#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <random>

class Engine
{
private:
    /* data */
public:
    int mWhitePawns;
    int mDepth;
    std::string mStartingPosition;
    std::string mLastBestMove;

    int mSetStartingPosition(std::string startingPosition);
    std::string mSearch();

    Engine();
    ~Engine();
};

class ZobristHashing {
private:
    std::unordered_map<int, std::vector<uint64_t>> pieceHashes;
    uint64_t currentHash;

public:
    ZobristHashing(int numPieces) {
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

    void movePiece(int piece, int from, int to) {
        currentHash ^= pieceHashes[piece][from];
        currentHash ^= pieceHashes[piece][to];
    }

    uint64_t getHash() const {
        return currentHash;
    }

    ZobristHashing();
    ~ZobristHashing();
};


#endif