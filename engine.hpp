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
    uint64_t mWhitePawns;
    uint64_t mWhiteRooks;
    uint64_t mWhiteKnights;
    uint64_t mWhiteBishops;
    uint64_t mWhiteKing;
    uint64_t mWhiteQueen;
    uint64_t mBlackPawns;
    uint64_t mBlackRooks;
    uint64_t mBlackKnights;
    uint64_t mBlackBishops;
    uint64_t mBlackKing;
    uint64_t mBlackQueen;

    bool mTurn;
    bool mWhiteAbleCastleKing;
    bool mWhiteAbleCastleQueen;
    bool mBlackAbleCastleKing;
    bool mBlackAbleCastleQueen;

    std::string mEnPassentSquare;
    int mHalfMove;
    int mFullMove;

    uint64_t knightAttacks[64];
    uint64_t kingAttacks[64];

    int mAttackGeneration(uint64_t piece);
    int mGetPseudoLegalMoves(uint64_t piece);
    int mFilterLegalMoves(uint64_t piece);
    int mVertHorMask(uint64_t piece);
    int mDiagonalMask(uint64_t piece);
    int mComputeKnightAttack();
    int mComputeKingAttack();

public:
    int mDepth;
    std::string mStartingPosition;
    std::string mLastBestMove;

    int mSetStartingPosition(std::string startingPosition);
    int mMovePiece(std::string move);
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