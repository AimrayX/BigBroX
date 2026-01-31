#include "position.hpp"

#include <iostream>
#include <sstream>

#include "utils.hpp"
#include "attack.hpp"

int Position::setStartingPosition(std::string startingPosition) {
        int realIndex = 0;
    for (int i = 0; i < 71; i++) {
        realIndex = (63-i) + (i-((63-i) % 8));
        if    (startingPosition[i] == '8' || startingPosition[i] == '7' 
            || startingPosition[i] == '6' || startingPosition[i] == '5' 
            || startingPosition[i] == '4' || startingPosition[i] == '3' 
            || startingPosition[i] == '2' || startingPosition[i] == '1') {
            i += startingPosition[i] - '0';
        } else if (startingPosition[i] == 'r') {
            mBlackRooks |= 1 << realIndex;
            board[realIndex] = Piece::B_ROOK;
        } else if (startingPosition[i] == 'n') {
            mBlackKnights |= 1 << realIndex;
            board[realIndex] = Piece::B_KNIGHT;
        } else if (startingPosition[i] == 'b') {
            mBlackBishops |= 1 << realIndex;
            board[realIndex] = Piece::B_BISHOP;
        } else if (startingPosition[i] == 'q') {
            mBlackQueen |= 1 << realIndex;
            board[realIndex] = Piece::B_QUEEN;
        } else if (startingPosition[i] == 'k') {
            mBlackKing |= 1 << realIndex;
            board[realIndex] = Piece::B_KING;
        } else if (startingPosition[i] == 'p') {
            mBlackPawns |= 1 << realIndex;
            board[realIndex] = Piece::B_PAWN;
        } else if (startingPosition[i] == 'R') {
            mWhiteRooks |= 1 << realIndex;
            board[realIndex] = Piece::W_ROOK;
        } else if (startingPosition[i] == 'N') {
            mWhiteKnights |= 1 << realIndex;
            board[realIndex] = Piece::W_KNIGHT;
        } else if (startingPosition[i] == 'B') {
            mWhiteBishops |= 1 << realIndex;
            board[realIndex] = Piece::W_BISHOP;
        } else if (startingPosition[i] == 'Q') {
            mWhiteQueen |= 1 << realIndex;
            board[realIndex] = Piece::W_QUEEN;
        } else if (startingPosition[i] == 'K') {
            mWhiteKing |= 1 << realIndex;
            board[realIndex] = Piece::W_KING;
        } else if (startingPosition[i] == 'P') {
            mWhitePawns |= 1 << realIndex;
            board[realIndex] = Piece::W_PAWN;
        }
        
    }
    std::istringstream iss(startingPosition);
    std::string temp;
    std::getline(iss, temp, ' ');

    std::getline(iss, temp, ' ');
    if (temp == "w") {
        mSideToMove = WHITE;
    } else if (temp == "b") {
        mSideToMove = BLACK;
    } else {
        mSideToMove = COLOR_NB;
    }

    std::getline(iss, temp, ' ');
    for (int i = 0; i < temp.length(); i++)
    {
        if (temp[i] == 'K') {
            mCastleRight = WHITE_OO;
        } else if (temp[i] == 'Q') {
            mCastleRight = WHITE_OOO;
        } else if (temp[i] == 'k') {
            mCastleRight = BLACK_OO;
        } else if (temp[i] == 'q') {
            mCastleRight = BLACK_OOO;
        }
    }

    std::getline(iss, temp, ' ');
    mEnPassentSquare = temp;
    
    std::getline(iss, temp, ' ');
    mHalfMove = std::stoi(temp);

    std::getline(iss, temp, ' ');
    mFullMove = std::stoi(temp); 

    while (iss >> temp) {
        doMove(util::parseUCIMove(temp));
    }

    return 0;
}

int Position::attackGeneration(Piece piece, int index) {
    uint64_t attack = 0;
    if ((piece | (Piece::W_ROOK | Piece::B_ROOK)) == (mWhiteBishops | mBlackBishops)) {
        attack = attack::vertHorMask(index);
    } else if ((piece | (mWhiteBishops | mBlackBishops)) == (mWhiteBishops | mBlackBishops)) {
        attack = attack::diagonalMask(index);
    } else if ((piece | (mWhiteQueen | mBlackQueen)) == (mWhiteQueen | mBlackQueen)) {
        attack = attack::diagonalMask(index) | attack::vertHorMask(piece);
    } else if ((piece | (mWhiteKing | mBlackKing)) == (mWhiteKing | mBlackKing)) {

        attack = attack::kingAttacks[index];
    }
    
    return attack;
}

int Position::getPseudoLegalMoves(uint64_t piece) {

}

int Position::filterLegalMoves(uint64_t piece) {

}

uint64_t getAllOccupiedSquares() {
    
}

void Position::doMove(Move m) {
}

void Position::undoMove(Move m) {


}

std::vector<Move> Position::getMoves() {


  return;
}

Position::Position(/* args */) {
}
Position::~Position() {
}
