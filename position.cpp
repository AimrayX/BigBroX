#include "position.hpp"

#include <cstdint>
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
    for (size_t i = 0; i < temp.length(); i++)
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
    mEnPassentSquare = util::mAlgebraicToBit(temp);
    
    std::getline(iss, temp, ' ');
    mHalfMove = std::stoi(temp);

    std::getline(iss, temp, ' ');
    mFullMove = std::stoi(temp); 

    while (iss >> temp) {
        doMove(util::parseUCIMove(temp));
    }

    return 0;
}

uint64_t Position::attackGeneration(int square, int type, Color color) {
    uint64_t attack = 0;
    if (type == ROOK) {
        attack = attack::getRookAttacks(square, occupancies[2]);
    } else if (type == BISHOP) {
        attack = attack::getBishopAttacks(square, occupancies[2]);
    } else if (type == QUEEN) {
        attack = (attack::getRookAttacks(square, occupancies[2]) | attack::getBishopAttacks(square, occupancies[2]));
    } else if (type == KING) {
        attack = attack::kingAttacks[square];
    } else if (type == KNIGHT) {
        attack = attack::knightAttacks[square];
    } else {
        attack = attack::getPawnAttacks(square, color, occupancies[~color]);
    }
    return attack;
}

uint64_t Position::getPseudoLegalMoves(int square, int type, Color color) {
  return (attackGeneration(square, type, color) & ~occupancies[color]);
}

void Position::doMove(Move m) {
  StateInfo state;

  state.castle = mCastleRight;
  state.epSquare = mEnPassentSquare;
  state.halfMove = mHalfMove;
  state.capturedPiece = NOPIECE;
  state.movedPiece = NOPIECE;

  uint64_t piece = (1ULL << m.from);
  
  if (piece & pieces[mSideToMove][ROOK]) {
    pieces[mSideToMove][ROOK] &= ~piece;
    pieces[mSideToMove][ROOK] |= m.to;
    state.movedPiece = ROOK;
  } else if (piece & pieces[mSideToMove][BISHOP]) {
    pieces[mSideToMove][BISHOP] &= ~piece;
    pieces[mSideToMove][BISHOP] |= m.to;
    state.movedPiece = BISHOP;
  } else if (piece & pieces[mSideToMove][QUEEN]) {
    pieces[mSideToMove][QUEEN] &= ~piece;
    pieces[mSideToMove][QUEEN] |= m.to;
    state.movedPiece = QUEEN;
  } else if (piece & pieces[mSideToMove][KING]) {
    pieces[mSideToMove][KING] &= ~piece;
    pieces[mSideToMove][KING] |= m.to;
    state.movedPiece = KING;
  } else if (piece & pieces[mSideToMove][KNIGHT]) {
    pieces[mSideToMove][KNIGHT] &= ~piece;
    pieces[mSideToMove][KNIGHT] |= m.to;
    state.movedPiece = KNIGHT;
  } else {
    pieces[mSideToMove][PAWN] &= ~piece;
    pieces[mSideToMove][m.promotion] |= m.to;
    state.movedPiece = PAWN;
    state.promotedToPiece = m.promotion;
    state.promotionSquare = m.to;
  }

  piece = (1ULL << m.to);
  mSideToMove = ((mSideToMove == WHITE) ? BLACK : WHITE);

  if (piece & pieces[mSideToMove][ROOK]) {
    pieces[mSideToMove][ROOK] &= ~piece;
    state.capturedPiece = ROOK;
  } else if (piece & pieces[mSideToMove][BISHOP]) {
    pieces[mSideToMove][BISHOP] &= ~piece;
    state.capturedPiece = BISHOP;
  } else if (piece & pieces[mSideToMove][QUEEN]) {
    pieces[mSideToMove][QUEEN] &= ~piece;
    state.capturedPiece = QUEEN;
  } else if (piece & pieces[mSideToMove][KING]) {
    pieces[mSideToMove][KING] &= ~piece;
    state.capturedPiece = KING;
  } else if (piece & pieces[mSideToMove][KNIGHT]) {
    pieces[mSideToMove][KNIGHT] &= ~piece;
    state.capturedPiece = KNIGHT;
  } else {
    pieces[mSideToMove][PAWN] &= ~piece;
    state.capturedPiece = PAWN;
  }

  history.push_back(state);
}

void Position::undoMove(Move m) {
  StateInfo oldState = history.back();
  history.pop_back();

  mCastleRight = oldState.castle;
  mEnPassentSquare = oldState.epSquare;
  mHalfMove = oldState.halfMove;

  if(oldState.capturedPiece != NOPIECE) {
    pieces[mSideToMove][oldState.capturedPiece] |= (1ULL << m.to);
  }

  mSideToMove = ((mSideToMove == WHITE) ? BLACK : WHITE);

  if(m.promotion != NOPIECE) {
    pieces[mSideToMove][oldState.movedPiece] &= ~(1ULL << m.to);
    pieces[mSideToMove][oldState.movedPiece] |= (1ULL << m.from);
  } else {
    pieces[mSideToMove][oldState.promotedToPiece] &= ~(1ULL << oldState.promotionSquare);
    if(mSideToMove) {
      pieces[mSideToMove][PAWN] |= ((1ULL << oldState.promotionSquare) << 8);
    } else {
      pieces[mSideToMove][PAWN] |= ((1ULL << oldState.promotionSquare) >> 8);
    }
  }
}

void Position::getMoves(Color color, std::vector<Move>& moveList) {
  for(int i = 0; i < 6; i++) {
    uint64_t piece = pieces[color][i];
    while(piece) {
      int sourceSquare = __builtin_ctzll(piece);
      uint64_t validTargets = getPseudoLegalMoves(sourceSquare, i, color);

      while(validTargets) {
        int targetSquare = __builtin_ctzll(validTargets);
        if(i == PAWN && (targetSquare/8) == ((color == WHITE) ? 7 : 0)) {
          moveList.push_back(Move(sourceSquare, targetSquare, QUEEN));
          moveList.push_back(Move(sourceSquare, targetSquare, KNIGHT));
          moveList.push_back(Move(sourceSquare, targetSquare, BISHOP));
          moveList.push_back(Move(sourceSquare, targetSquare, ROOK));
        } else {
          moveList.push_back(Move(sourceSquare, targetSquare, NOPIECE));
        }
        validTargets &= (validTargets - 1);
      }
      piece &= (piece - 1);
    }
  }
}

Position::Position(/* args */) {
}
Position::~Position() {
}
