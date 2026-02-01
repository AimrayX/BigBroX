#include "position.hpp"

#include <sstream>

#include "types.hpp"
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
            pieces[BLACK][ROOK] |= 1 << realIndex;
            occupancies[BLACK] |= pieces[BLACK][ROOK];
        } else if (startingPosition[i] == 'n') {
            pieces[BLACK][KNIGHT] |= 1 << realIndex;
            occupancies[BLACK] |= pieces[BLACK][KNIGHT];
        } else if (startingPosition[i] == 'b') {
            pieces[BLACK][BISHOP] |= 1 << realIndex;
            occupancies[BLACK] |= pieces[BLACK][BISHOP];
        } else if (startingPosition[i] == 'q') {
            pieces[BLACK][QUEEN] |= 1 << realIndex;
            occupancies[BLACK] |= pieces[BLACK][QUEEN];
        } else if (startingPosition[i] == 'k') {
            pieces[BLACK][KING] |= 1 << realIndex;
            occupancies[BLACK] |= pieces[BLACK][KING];
        } else if (startingPosition[i] == 'p') {
            pieces[BLACK][PAWN] |= 1 << realIndex;
            occupancies[BLACK] |= pieces[BLACK][PAWN];
        } else if (startingPosition[i] == 'R') {
            pieces[WHITE][ROOK] |= 1 << realIndex;
            occupancies[WHITE] |= pieces[WHITE][ROOK];
        } else if (startingPosition[i] == 'N') {
            pieces[WHITE][KNIGHT] |= 1 << realIndex;
            occupancies[WHITE] |= pieces[WHITE][KNIGHT];
        } else if (startingPosition[i] == 'B') {
            pieces[WHITE][BISHOP] |= 1 << realIndex;
            occupancies[WHITE] |= pieces[WHITE][BISHOP];
        } else if (startingPosition[i] == 'Q') {
            pieces[WHITE][QUEEN] |= 1 << realIndex;
            occupancies[WHITE] |= pieces[WHITE][QUEEN];
        } else if (startingPosition[i] == 'K') {
            pieces[WHITE][KING] |= 1 << realIndex;
            occupancies[WHITE] |= pieces[WHITE][KING];
        } else if (startingPosition[i] == 'P') {
            pieces[WHITE][PAWN] |= 1 << realIndex;
            occupancies[WHITE] |= pieces[WHITE][PAWN];
        }
      occupancies[2] |= occupancies[BLACK] | occupancies[WHITE];
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

int Position::attackGeneration(uint64_t piece) {
    uint64_t attack = 0;
    if ((piece & pieces[BLACK][ROOK]) || (piece & pieces[WHITE][BISHOP]) != 0) {
        attack = attack::vertHorMask(piece);
    } else if ((piece & pieces[BLACK][BISHOP]) || (piece & pieces[WHITE][BISHOP]) != 0) {
        attack = attack::diagonalMask(piece);
    } else if ((piece & pieces[BLACK][QUEEN]) || (piece & pieces[WHITE][QUEEN]) != 0) {
        attack = (attack::diagonalMask(piece) | attack::vertHorMask(piece));
    } else if ((piece & pieces[BLACK][KING]) || (piece & pieces[WHITE][KING]) != 0) {
        attack = attack::kingAttacks[piece];
    }
    return attack;
}

int Position::getPseudoLegalMoves(uint64_t piece) {
  uint64_t pseudoLegalMoves = 0;
  pseudoLegalMoves = attackGeneration(piece);


  return pseudoLegalMoves;
}

int Position::filterLegalMoves(uint64_t piece) {
  
}

uint64_t getAllOccupiedSquares() {
    
}

void Position::doMove(Move m) {
}

void Position::undoMove(Move m) {


}

std::vector<Move> Position::getMoves(Color color) {
  std::vector<Move> moves;
  for(int i = 0; i < 6; i++) {
    uint64_t piece = pieces[color][i];
    while(piece) {
    int square = __builtin_ctzll(piece);
    Position::getPseudoLegalMoves(square);


    piece &= (piece -1);
    }
  }
  return moves;
}

Position::Position(/* args */) {
}
Position::~Position() {
}
