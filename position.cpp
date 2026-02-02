#include "position.hpp"

#include <cstdint>
#include <sstream>
#include <iostream>

#include "types.hpp"
#include "utils.hpp"
#include "attack.hpp"

int Position::setStartingPosition(std::string startingPosition) {
    // Clear existing state first
    for(int c = 0; c < 2; c++) {
        for(int p = 0; p < 6; p++) {
            pieces[c][p] = 0ULL;
        }
    }
    occupancies[0] = 0ULL; 
    occupancies[1] = 0ULL; 
    occupancies[2] = 0ULL;

    int rank = 7;
    int file = 0;

    // We only iterate the board part of the FEN (stop at space)
    std::istringstream iss(startingPosition);
    std::string boardPart;
    std::getline(iss, boardPart, ' ');

    for (char c : boardPart) {
        if (c == '/') {
            rank--;
            file = 0;
        } 
        else if (isdigit(c)) {
            // Advance the file by the number value (empty squares)
            file += (c - '0');
        } 
        else {
            // It's a piece
            int square = rank * 8 + file;
            int type = NOPIECE;
            int color = (isupper(c)) ? WHITE : BLACK;
            char lowerC = tolower(c);

            if (lowerC == 'p') type = PAWN;
            else if (lowerC == 'n') type = KNIGHT;
            else if (lowerC == 'b') type = BISHOP;
            else if (lowerC == 'r') type = ROOK;
            else if (lowerC == 'q') type = QUEEN;
            else if (lowerC == 'k') type = KING;

            if (type != NOPIECE) {
                pieces[color][type] |= (1ULL << square);
                occupancies[color] |= (1ULL << square);
            }
            file++;
        }
    }

    occupancies[2] = occupancies[WHITE] | occupancies[BLACK];

    // Continue with the rest of the parsing (Side to move, castling, etc.)
    std::string temp;

    if (std::getline(iss, temp, ' ')) {
         if (temp == "w") {
            mSideToMove = WHITE;
        } else if (temp == "b") {
            mSideToMove = BLACK;
        }
    }

    mCastleRight = 0;
    if (std::getline(iss, temp, ' ')) {
        for (char c : temp) {
            if (c == 'K') mCastleRight |= WHITE_OO;
            else if (c == 'Q') mCastleRight |= WHITE_OOO;
            else if (c == 'k') mCastleRight |= BLACK_OO;
            else if (c == 'q') mCastleRight |= BLACK_OOO;
        }
    }

    // En Passant
    if (std::getline(iss, temp, ' ')) {
        mEnPassentSquare = util::mAlgebraicToBit(temp);
    }

    // Half/Full moves
    if (std::getline(iss, temp, ' ')) mHalfMove = std::stoi(temp);
    if (std::getline(iss, temp, ' ')) mFullMove = std::stoi(temp); 

    printBoard();

    // Handle additional moves (UCI "position startpos moves e2e4 ...")
    while (iss >> temp) {
        doMove(util::parseUCIMove(temp));
    }

    printBoard();

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
        uint64_t extendedOccupancy = occupancies[2];
        if(mEnPassentSquare != 0) {
          extendedOccupancy |= mEnPassentSquare;
        }
        attack = attack::getPawnAttacks(square, color, extendedOccupancy);
        mAttacksP = attack;
    }
    return attack;
}

uint64_t Position::getPseudoLegalMoves(int square, int type, Color color) {
  return (attackGeneration(square, type, color) & ~occupancies[color]);
}

// Check if 'square' is attacked by pieces of 'sideAttacking'
bool Position::isSquareAttacked(int square, Color sideAttacking) {
    Color defendingSide = (sideAttacking == WHITE) ? BLACK : WHITE;
    if (attack::getPawnAttacks(square, defendingSide, occupancies[2]) & pieces[sideAttacking][PAWN]) {
        return true;
    }

    if (attack::knightAttacks[square] & pieces[sideAttacking][KNIGHT]) {
        return true;
    }

    if (attack::kingAttacks[square] & pieces[sideAttacking][KING]) {
        return true;
    }

    uint64_t bishopsQueens = pieces[sideAttacking][BISHOP] | pieces[sideAttacking][QUEEN];
    if (attack::getBishopAttacks(square, occupancies[2]) & bishopsQueens) {
        return true;
    }

    uint64_t rooksQueens = pieces[sideAttacking][ROOK] | pieces[sideAttacking][QUEEN];
    if (attack::getRookAttacks(square, occupancies[2]) & rooksQueens) {
        return true;
    }

    return false;
}

void Position::doMove(Move m) {
    StateInfo state;

    state.castle = mCastleRight;
    state.epSquare = mEnPassentSquare;
    state.halfMove = mHalfMove;
    state.capturedPiece = NOPIECE;
    state.movedPiece = NOPIECE;

    uint64_t startBit = (1ULL << m.from);
    uint64_t endBit   = (1ULL << m.to); 

    if (startBit & pieces[mSideToMove][ROOK]) {
        pieces[mSideToMove][ROOK] &= ~startBit;
        pieces[mSideToMove][ROOK] |= endBit; 
        state.movedPiece = ROOK;
        mEnPassentSquare = 0;
    } 
    else if (startBit & pieces[mSideToMove][BISHOP]) {
        pieces[mSideToMove][BISHOP] &= ~startBit;
        pieces[mSideToMove][BISHOP] |= endBit;
        state.movedPiece = BISHOP;
        mEnPassentSquare = 0;
    } 
    else if (startBit & pieces[mSideToMove][QUEEN]) {
        pieces[mSideToMove][QUEEN] &= ~startBit;
        pieces[mSideToMove][QUEEN] |= endBit;
        state.movedPiece = QUEEN;
        mEnPassentSquare = 0;
    } 
    else if (startBit & pieces[mSideToMove][KING]) {
        pieces[mSideToMove][KING] &= ~startBit;
        pieces[mSideToMove][KING] |= endBit;
        state.movedPiece = KING;

        if ((int)m.to - (int)m.from == 2) { 
            uint64_t rookFrom = (1ULL << (m.to + 1));
            uint64_t rookTo   = (1ULL << (m.to - 1));

            pieces[mSideToMove][ROOK] &= ~rookFrom;
            pieces[mSideToMove][ROOK] |= rookTo;
        } 
        else if ((int)m.to - (int)m.from == -2) {
            uint64_t rookFrom = (1ULL << (m.to - 2));
            uint64_t rookTo   = (1ULL << (m.to + 1));

            pieces[mSideToMove][ROOK] &= ~rookFrom;
            pieces[mSideToMove][ROOK] |= rookTo;
        }
    } 
    else if (startBit & pieces[mSideToMove][KNIGHT]) {
        pieces[mSideToMove][KNIGHT] &= ~startBit;
        pieces[mSideToMove][KNIGHT] |= endBit;
        state.movedPiece = KNIGHT;
        mEnPassentSquare = 0;
    } else {
        pieces[mSideToMove][PAWN] &= ~startBit;

        if (m.promotion == NOPIECE) {
            pieces[mSideToMove][PAWN] |= endBit;
        } else {
            pieces[mSideToMove][m.promotion] |= endBit;
        }

        if(endBit == mEnPassentSquare) {
          state.capturedPiece = PAWN;

          int capturedOffset = (mSideToMove == WHITE) ? -8 : 8;
          uint64_t enemyPawnLoc = (1ULL << (m.to + capturedOffset));

          pieces[(mSideToMove == WHITE ? BLACK : WHITE)][PAWN] &= ~enemyPawnLoc;
        }

        if (abs(m.to - m.from) == 16) {
          int skippedSquare = (m.from + m.to) / 2;
          mEnPassentSquare = (1ULL << skippedSquare); 
        } else {
          mEnPassentSquare = 0;
        }

        state.movedPiece = PAWN;
        state.promotionSquare = m.to;
    }

    mSideToMove = ((mSideToMove == WHITE) ? BLACK : WHITE);

    if(state.capturedPiece == NOPIECE) {
      if (endBit & pieces[mSideToMove][ROOK]) {
        pieces[mSideToMove][ROOK] &= ~endBit;
        state.capturedPiece = ROOK;
      } else if (endBit & pieces[mSideToMove][BISHOP]) {
        pieces[mSideToMove][BISHOP] &= ~endBit;
        state.capturedPiece = BISHOP;
      } else if (endBit & pieces[mSideToMove][QUEEN]) {
        pieces[mSideToMove][QUEEN] &= ~endBit;
        state.capturedPiece = QUEEN;
      } else if (endBit & pieces[mSideToMove][KING]) {
        pieces[mSideToMove][KING] &= ~endBit;
        state.capturedPiece = KING;
      } else if (endBit & pieces[mSideToMove][KNIGHT]) {
        pieces[mSideToMove][KNIGHT] &= ~endBit;
        state.capturedPiece = KNIGHT;
      } else if (endBit & pieces[mSideToMove][PAWN]) {
        pieces[mSideToMove][PAWN] &= ~endBit;
        state.capturedPiece = PAWN;
      }
    }
    occupancies[WHITE] = pieces[WHITE][PAWN] | pieces[WHITE][KNIGHT] | pieces[WHITE][BISHOP] | pieces[WHITE][QUEEN] | pieces[WHITE][KING] | pieces[WHITE][ROOK];
    occupancies[BLACK] = pieces[BLACK][PAWN] | pieces[BLACK][KNIGHT] | pieces[BLACK][BISHOP] | pieces[BLACK][QUEEN] | pieces[BLACK][KING] | pieces[BLACK][ROOK];
    occupancies[2] = occupancies[WHITE] | occupancies[BLACK];

    history.push_back(state);
}

void Position::undoMove(Move m) {
  StateInfo oldState = history.back();
  history.pop_back();

  mCastleRight = oldState.castle;
  mEnPassentSquare = oldState.epSquare;
  mHalfMove = oldState.halfMove;

    mSideToMove = (mSideToMove == WHITE) ? BLACK : WHITE;

    if (m.promotion != NOPIECE) {
        pieces[mSideToMove][m.promotion] &= ~(1ULL << m.to);
        pieces[mSideToMove][PAWN] |= (1ULL << m.from);
    } 
    else {
        pieces[mSideToMove][oldState.movedPiece] &= ~(1ULL << m.to);
        pieces[mSideToMove][oldState.movedPiece] |= (1ULL << m.from);

        if(oldState.movedPiece == KING) {
          if((int)m.to - (int)m.from == 2) {
            uint64_t rookFrom = (1ULL << (m.to + 1));
            uint64_t rookTo = (1ULL << (m.to - 1));
            pieces[mSideToMove][ROOK] &= ~rookTo;
            pieces[mSideToMove][ROOK] |= rookFrom;

          } else if((int)m.to - (int)m.from == -2) {
            uint64_t rookFrom = (1ULL << (m.to - 2));
            uint64_t rookTo = (1ULL << (m.to + 1));
            pieces[mSideToMove][ROOK] &= ~rookTo;
            pieces[mSideToMove][ROOK] |= rookFrom;

          }
        }
    }

    if (oldState.capturedPiece != NOPIECE) {
        Color enemy = (mSideToMove == WHITE) ? BLACK : WHITE;

        if(oldState.movedPiece == PAWN && (1ULL << m.to) == oldState.epSquare) {
          int captureOffset = (mSideToMove == WHITE) ? -8 : 8;
          pieces[enemy][PAWN] |= (1ULL << (m.to + captureOffset));

        } else {
          pieces[enemy][oldState.capturedPiece] |= (1ULL << m.to);
        }
    }

    // Update bitboards
    occupancies[WHITE] = pieces[WHITE][PAWN] | pieces[WHITE][KNIGHT] |  pieces[WHITE][BISHOP] | pieces[WHITE][QUEEN] | pieces[WHITE][KING] | pieces[WHITE][ROOK];

    occupancies[BLACK] = pieces[BLACK][PAWN] | pieces[BLACK][KNIGHT] |  pieces[BLACK][BISHOP] | pieces[BLACK][QUEEN] | pieces[BLACK][KING] | pieces[BLACK][ROOK];

    occupancies[2] = occupancies[WHITE] | occupancies[BLACK];

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

void Position::printBoard() {
    std::cout << "   occupancies both" << std::endl;
    std::cout << "  +-----------------+" << std::endl;
    // Iterate Ranks from 7 (Row 8) down to 0 (Row 1)
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << rank + 1 << " | "; // Print Rank Number
        for (int file = 0; file < 8; file++) {
            // Calculate the actual square index (0 to 63)
            int square = rank * 8 + file;
            
            // Check the bit at 'square'
            if ((occupancies[2] >> square) & 1) {
                std::cout << "1 ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << "|" << std::endl;
    }
    std::cout << "  +-----------------+" << std::endl;
    std::cout << "    a b c d e f g h" << std::endl;
    /*
    std::cout << "   attacks pawns" << std::endl;
    std::cout << "  +-----------------+" << std::endl;
    uint64_t attacksP = mAttacksP;
    // Iterate Ranks from 7 (Row 8) down to 0 (Row 1)
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << rank + 1 << " | "; // Print Rank Number
        for (int file = 0; file < 8; file++) {
            // Calculate the actual square index (0 to 63)
            int square = rank * 8 + file;
            
            // Check the bit at 'square'
            if ((attacksP >> square) & 1) {
                std::cout << "1 ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << "|" << std::endl;
    }
    std::cout << "  +-----------------+" << std::endl;
    std::cout << "    a b c d e f g h" << std::endl;
    */
}

Position::Position(/* args */) {
}
Position::~Position() {
}
