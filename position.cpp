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

    for(int k = 0; k < 64; k++) {
      board[k] = 0;
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
                board[square] = type;
            }
            file++;
        }
    }

    occupancies[2] = occupancies[WHITE] | occupancies[BLACK];

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
        board[m.from] = 0;
        board[m.to] = ROOK;
        state.movedPiece = ROOK;
        mEnPassentSquare = 0;
    } 
    else if (startBit & pieces[mSideToMove][BISHOP]) {
        pieces[mSideToMove][BISHOP] &= ~startBit;
        pieces[mSideToMove][BISHOP] |= endBit;
        board[m.from] = 0;
        board[m.to] = BISHOP;
        state.movedPiece = BISHOP;
        mEnPassentSquare = 0;
    } 
    else if (startBit & pieces[mSideToMove][QUEEN]) {
        pieces[mSideToMove][QUEEN] &= ~startBit;
        pieces[mSideToMove][QUEEN] |= endBit;
        board[m.from] = 0;
        board[m.to] = QUEEN;
        state.movedPiece = QUEEN;
        mEnPassentSquare = 0;
    } 
    else if (startBit & pieces[mSideToMove][KING]) {
        pieces[mSideToMove][KING] &= ~startBit;
        pieces[mSideToMove][KING] |= endBit;
        board[m.from] = 0;
        board[m.to] = KING;
        state.movedPiece = KING;

        if ((int)m.to - (int)m.from == 2) { 
            uint64_t rookFrom = (1ULL << (m.to + 1));
            uint64_t rookTo   = (1ULL << (m.to - 1));

            pieces[mSideToMove][ROOK] &= ~rookFrom;
            pieces[mSideToMove][ROOK] |= rookTo;
            board[m.from] = 0;
            board[m.to] = ROOK;
        } 
        else if ((int)m.to - (int)m.from == -2) {
            uint64_t rookFrom = (1ULL << (m.to - 2));
            uint64_t rookTo   = (1ULL << (m.to + 1));

            pieces[mSideToMove][ROOK] &= ~rookFrom;
            pieces[mSideToMove][ROOK] |= rookTo;
            board[m.from] = 0;
            board[m.to] = ROOK;
        }
    } 
    else if (startBit & pieces[mSideToMove][KNIGHT]) {
        pieces[mSideToMove][KNIGHT] &= ~startBit;
        pieces[mSideToMove][KNIGHT] |= endBit;
        board[m.from] = 0;
        board[m.to] = KNIGHT;
        state.movedPiece = KNIGHT;
        mEnPassentSquare = 0;
    } else {
        pieces[mSideToMove][PAWN] &= ~startBit;
        board[m.from] = 0;

        if (m.promotion == NOPIECE) {
            pieces[mSideToMove][PAWN] |= endBit;
            board[m.to] = PAWN;
        } else {
            pieces[mSideToMove][m.promotion] |= endBit;
            board[m.to] = m.promotion;
        }

        if(endBit == mEnPassentSquare) {
          state.capturedPiece = PAWN;

          int capturedOffset = (mSideToMove == WHITE) ? -8 : 8;
          uint64_t enemyPawnLoc = (1ULL << (m.to + capturedOffset));

          pieces[(mSideToMove == WHITE ? BLACK : WHITE)][PAWN] &= ~enemyPawnLoc;
          board[m.to + capturedOffset] = 0;
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

    // --- UPDATE CASTLING RIGHTS ---
    // 1. If King moves, lose both rights
    if (state.movedPiece == KING) {
        if (mSideToMove == BLACK) mCastleRight &= ~(WHITE_OO | WHITE_OOO); 
        else                      mCastleRight &= ~(BLACK_OO | BLACK_OOO);
    }

    // 2. If Rook moves, lose rights for that side
    if (state.movedPiece == ROOK) {
        if (m.from == 0)       mCastleRight &= ~WHITE_OOO;
        else if (m.from == 7)  mCastleRight &= ~WHITE_OO;
        else if (m.from == 56) mCastleRight &= ~BLACK_OOO;
        else if (m.from == 63) mCastleRight &= ~BLACK_OO;
    }

    // 3. If Rook is captured, opponent loses rights for that side
    if (state.capturedPiece == ROOK) {
        if (m.to == 0)       mCastleRight &= ~WHITE_OOO;
        else if (m.to == 7)  mCastleRight &= ~WHITE_OO;
        else if (m.to == 56) mCastleRight &= ~BLACK_OOO;
        else if (m.to == 63) mCastleRight &= ~BLACK_OO;
    }

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
        board[m.to] = 0;
        board[m.from] = PAWN;
    } 
    else {
        pieces[mSideToMove][oldState.movedPiece] &= ~(1ULL << m.to);
        pieces[mSideToMove][oldState.movedPiece] |= (1ULL << m.from);
        board[m.to] = 0;
        board[m.from] = oldState.movedPiece;

        if(oldState.movedPiece == KING) {
          if((int)m.to - (int)m.from == 2) {
            uint64_t rookFrom = (1ULL << (m.to + 1));
            uint64_t rookTo = (1ULL << (m.to - 1));
            pieces[mSideToMove][ROOK] &= ~rookTo;
            pieces[mSideToMove][ROOK] |= rookFrom;
            board[m.to - 1] = 0;
            board[m.to + 1] = ROOK;

          } else if((int)m.to - (int)m.from == -2) {
            uint64_t rookFrom = (1ULL << (m.to - 2));
            uint64_t rookTo = (1ULL << (m.to + 1));
            pieces[mSideToMove][ROOK] &= ~rookTo;
            pieces[mSideToMove][ROOK] |= rookFrom;
            board[m.to + 1] = 0;
            board[m.to - 2] = ROOK;

          }
        }
    }

    if (oldState.capturedPiece != NOPIECE) {
        Color enemy = (mSideToMove == WHITE) ? BLACK : WHITE;

        if(oldState.movedPiece == PAWN && (1ULL << m.to) == oldState.epSquare) {
          int captureOffset = (mSideToMove == WHITE) ? -8 : 8;
          pieces[enemy][PAWN] |= (1ULL << (m.to + captureOffset));
          board[m.to + captureOffset] = PAWN;

        } else {
          pieces[enemy][oldState.capturedPiece] |= (1ULL << m.to);
          board[m.to] = oldState.capturedPiece;
        }
    }

    // Update bitboards
    occupancies[WHITE] = pieces[WHITE][PAWN] | pieces[WHITE][KNIGHT] |  pieces[WHITE][BISHOP] | pieces[WHITE][QUEEN] | pieces[WHITE][KING] | pieces[WHITE][ROOK];

    occupancies[BLACK] = pieces[BLACK][PAWN] | pieces[BLACK][KNIGHT] |  pieces[BLACK][BISHOP] | pieces[BLACK][QUEEN] | pieces[BLACK][KING] | pieces[BLACK][ROOK];

    occupancies[2] = occupancies[WHITE] | occupancies[BLACK];

}

void Position::getMoves(Color color, MoveList& moveList) {
  Color enemy = (color == WHITE) ? BLACK : WHITE;
  uint64_t enemyKing = pieces[enemy][KING];
  if (enemyKing == 0) {
    std::cout << "CRITICAL ERROR: Enemy King missing for color " << enemy << std::endl;
  }

  for(int i = 0; i < 6; i++) {
    uint64_t piece = pieces[color][i];
    while(piece) {
      int sourceSquare = __builtin_ctzll(piece);
      uint64_t validTargets = getPseudoLegalMoves(sourceSquare, i, color);
      validTargets &= ~enemyKing;

      while(validTargets) {
        int targetSquare = __builtin_ctzll(validTargets);
        int victim = board[targetSquare];
        int aggressor = i;
        int score = 0;

        if(victim != NOPIECE) {
          static const int victimScores[] = { 100, 300, 310, 500, 900, 0 };
          score = victimScores[victim] - victimScores[aggressor] + 10000;
        }

        if(i == PAWN && (targetSquare/8) == ((color == WHITE) ? 7 : 0)) {
          moveList.add(Move(sourceSquare, targetSquare, QUEEN), score);
          moveList.add(Move(sourceSquare, targetSquare, KNIGHT), score);
          moveList.add(Move(sourceSquare, targetSquare, BISHOP), score);
          moveList.add(Move(sourceSquare, targetSquare, ROOK), score);
        } else {
          moveList.add(Move(sourceSquare, targetSquare, NOPIECE), score);
        }
        validTargets &= (validTargets - 1);
      }
      piece &= (piece - 1);
    }
  }
  if (color == WHITE) {
        if ((mCastleRight & WHITE_OO) && 
            !(occupancies[2] & ((1ULL << 5) | (1ULL << 6)))) {
            if (!isSquareAttacked(4, BLACK) && 
                !isSquareAttacked(5, BLACK) && 
                !isSquareAttacked(6, BLACK)) {
                moveList.add(Move(4, 6, NOPIECE), 0); // e1g1
            }
        }

        if ((mCastleRight & WHITE_OOO) && 
            !(occupancies[2] & ((1ULL << 1) | (1ULL << 2) | (1ULL << 3)))) {
            if (!isSquareAttacked(4, BLACK) && 
                !isSquareAttacked(3, BLACK) && 
                !isSquareAttacked(2, BLACK)) {
                moveList.add(Move(4, 2, NOPIECE), 0); // e1c1
            }
        }
    } 
    else {
        if ((mCastleRight & BLACK_OO) && 
            !(occupancies[2] & ((1ULL << 61) | (1ULL << 62)))) {
            if (!isSquareAttacked(60, WHITE) && 
                !isSquareAttacked(61, WHITE) && 
                !isSquareAttacked(62, WHITE)) {
                moveList.add(Move(60, 62, NOPIECE), 0); // e8g8
            }
        }

        if ((mCastleRight & BLACK_OOO) && 
            !(occupancies[2] & ((1ULL << 57) | (1ULL << 58) | (1ULL << 59)))) {
            if (!isSquareAttacked(60, WHITE) && 
                !isSquareAttacked(59, WHITE) && 
                !isSquareAttacked(58, WHITE)) {
                moveList.add(Move(60, 58, NOPIECE), 0); // e8c8
            }
        }
    }
}

void Position::printBoard() {
    std::cout << "   occupancies both" << std::endl;
    std::cout << "  +-----------------+" << std::endl;
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << rank + 1 << " | "; // Print Rank Number
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
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
}

Position::Position() {
}
Position::~Position() {
}
