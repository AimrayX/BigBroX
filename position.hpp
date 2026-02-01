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

    int attackGeneration(uint64_t index);
    int getPseudoLegalMoves(uint64_t piece);
    int filterLegalMoves(uint64_t piece);
    int setStartingPosition(std::string startingPosition);
    uint64_t getAllOccupiedSquares();
    void doMove(Move m);
    void undoMove(Move m);
    std::vector<Move> getMoves(Color color);

    Position(/* args */);
    ~Position();
};



#endif
