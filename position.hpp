#ifndef POSITION_HPP
#define POSITION_HPP

#include <cstdint>
#include <stdint.h>
#include <string>
#include <vector>

#include "types.hpp"


class Position {

private:
    
public:
    std::string mStartingPosition;

    uint32_t mNumberOfWhitePieces;
    uint32_t mNumberOfBlackPieces;

    Color mSideToMove;
    int mCastleRight;

    uint64_t mEnPassentSquare;
    int mHalfMove;
    int mFullMove;

    uint64_t pieces[2][6];
    uint64_t occupancies[3];
    int board[64];
    
    uint64_t mHash;
    int mPosScore;

    uint64_t getHash() const { return mHash; }

    std::vector<StateInfo> history;

    uint64_t attackGeneration(int square, int type, Color color);
    uint64_t getPseudoLegalMoves(int piece, int type, Color color);
    int setStartingPosition(std::string startingPosition);
    int getPieceValue(int piece, int square, Color color);
    void doMove(Move m);
    void undoMove(Move m);
    void getMoves(Color color, MoveList& moveList);
    void getCaptures(Color color, MoveList& moveList);
    bool isSquareAttacked(int square, Color sideAttacking);
    bool isRepetition();

    void printBoard();


    uint64_t mAttacksP = 0ULL;

    Position();
    ~Position();
};



#endif
