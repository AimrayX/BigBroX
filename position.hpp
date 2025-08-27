#ifndef POSITION_HPP
#define POSITION_HPP

#include <stdint.h>
#include <string>
#include "types.hpp"

class Position {

private:
    
public:
    std::string mStartingPosition;

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

    Color mSideToMove;
    CastlingRights mCastleRight;

    std::string mEnPassentSquare;
    int mHalfMove;
    int mFullMove;

    uint64_t knightAttacks[64];
    uint64_t kingAttacks[64];
    Piece board[64];

    int attackGeneration(uint64_t piece);
    int getPseudoLegalMoves(uint64_t piece);
    int filterLegalMoves(uint64_t piece);
    int setStartingPosition(std::string startingPosition);
    void doMove(Move m);

    Position(/* args */);
    ~Position();
};



#endif