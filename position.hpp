#ifndef POSITION_HPP
#define POSITION_HPP

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
    CastlingRights mCastleRight;

    std::string mEnPassentSquare;
    int mHalfMove;
    int mFullMove;

    uint64_t pieces[2][6];
    uint64_t occupancies[3];

    uint64_t attackGeneration(int square, int type, Color color);
    uint64_t getPseudoLegalMoves(uint64_t piece, int type, Color color);
    int setStartingPosition(std::string startingPosition);
    void doMove(Move m);
    void undoMove(Move m);
    void getMoves(Color color, std::vector<Move>& moveList);

    Position(/* args */);
    ~Position();
};



#endif
