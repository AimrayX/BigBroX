#include "position.hpp"

#include <cstdint>
#include <iostream>
#include <random>
#include <sstream>

#include "attack.hpp"
#include "types.hpp"
#include "utils.hpp"

const uint64_t NOT_A_FILE = 0xFEFEFEFEFEFEFEFEULL;
const uint64_t NOT_H_FILE = 0x7F7F7F7F7F7F7F7FULL;

int PST_CACHE[2][6][64];  // Definition

void initPST() {
  for (int p = 0; p < 6; p++) {
    for (int s = 0; s < 64; s++) {
      // White: Direct copy
      PST_CACHE[WHITE][p][s] = pieceValues[p] + PST[p][s];

      // Black: Mirror and Negate
      // Note: We bake the piece value into the table too!
      // This saves the "val + PST..." addition at runtime.
      PST_CACHE[BLACK][p][s] = -(pieceValues[p] + PST[p][s ^ 56]);
    }
  }
}

namespace Zobrist {
uint64_t pieceKeys[2][6][64];  // [Color][Piece][Square]
uint64_t enPassantKeys[64];    // [Square]
uint64_t castleKeys[16];       // [CastleRights Mask]
uint64_t sideKey;              // XORed if Black to move
bool initialized = false;

void init() {
  if (initialized) return;
  std::mt19937_64 gen(123456789);  // Fixed seed for reproducibility
  std::uniform_int_distribution<uint64_t> dist;

  for (int c = 0; c < 2; c++) {
    for (int p = 0; p < 6; p++) {
      for (int s = 0; s < 64; s++) {
        pieceKeys[c][p][s] = dist(gen);
      }
    }
  }

  for (int i = 0; i < 64; i++) enPassantKeys[i] = dist(gen);
  for (int i = 0; i < 16; i++) castleKeys[i] = dist(gen);
  sideKey = dist(gen);
  initialized = true;
}
}  // namespace Zobrist

bool Position::isRepetition() {
  int n = gamePly;

  for (int i = n - 2; i >= 0 && i >= n - mHalfMove; i -= 2) {
    if (history[i].zobristKey == mHash) {
      return true;
    }
  }

  return false;
}

int Position::getPieceValue(int piece, int square, Color color) {
  return PST_CACHE[color][piece][square];
}

int Position::setStartingPosition(std::string startingPosition) {
  // Clear existing state first
  posEval.positionScore = 0;
  posEval.pawnStructureScore = 0;
  posEval.pieceMobilityScore = 0;

  gamePly = 0;

  for (int c = 0; c < 2; c++) {
    for (int p = 0; p < 6; p++) {
      pieces[c][p] = 0ULL;
    }
  }

  for (int k = 0; k < 64; k++) {
    board[k] = NOPIECE;
  }

  occupancies[0] = 0ULL;
  occupancies[1] = 0ULL;
  occupancies[2] = 0ULL;

  mHash = 0ULL;

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
    } else if (isdigit(c)) {
      // Advance the file by the number value (empty squares)
      file += (c - '0');
    } else {
      // It's a piece
      int square = rank * 8 + file;
      int type = NOPIECE;
      int color = (isupper(c)) ? WHITE : BLACK;
      char lowerC = tolower(c);

      if (lowerC == 'p')
        type = PAWN;
      else if (lowerC == 'n')
        type = KNIGHT;
      else if (lowerC == 'b')
        type = BISHOP;
      else if (lowerC == 'r')
        type = ROOK;
      else if (lowerC == 'q')
        type = QUEEN;
      else if (lowerC == 'k')
        type = KING;

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
      if (c == 'K')
        mCastleRight |= WHITE_OO;
      else if (c == 'Q')
        mCastleRight |= WHITE_OOO;
      else if (c == 'k')
        mCastleRight |= BLACK_OO;
      else if (c == 'q')
        mCastleRight |= BLACK_OOO;
    }
  }

  // En Passant
  if (std::getline(iss, temp, ' ')) {
    mEnPassentSquare = util::mAlgebraicToBit(temp);
  }

  // Half/Full moves
  if (std::getline(iss, temp, ' ')) mHalfMove = std::stoi(temp);
  if (std::getline(iss, temp, ' ')) mFullMove = std::stoi(temp);

  // 1. Pieces
  for (int p = 0; p < 6; p++) {
    uint64_t w = pieces[WHITE][p];
    while (w) {
      int sq = __builtin_ctzll(w);
      posEval.positionScore += getPieceValue(p, sq, WHITE);
      mHash ^= Zobrist::pieceKeys[WHITE][p][sq];
      w &= (w - 1);
    }
    uint64_t b = pieces[BLACK][p];
    while (b) {
      int sq = __builtin_ctzll(b);
      posEval.positionScore += getPieceValue(p, sq, BLACK);
      mHash ^= Zobrist::pieceKeys[BLACK][p][sq];
      b &= (b - 1);
    }
  }

  // 2. En Passant
  if (mEnPassentSquare) {
    int sq = __builtin_ctzll(mEnPassentSquare);
    mHash ^= Zobrist::enPassantKeys[sq];
  }

  // 3. Castle Rights
  mHash ^= Zobrist::castleKeys[mCastleRight];

  // 4. Side to Move
  if (mSideToMove == BLACK) {
    mHash ^= Zobrist::sideKey;
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
    attack = (attack::getRookAttacks(square, occupancies[2]) |
              attack::getBishopAttacks(square, occupancies[2]));
  } else if (type == KING) {
    attack = attack::kingAttacks[square];
  } else if (type == KNIGHT) {
    attack = attack::knightAttacks[square];
  } else {
    uint64_t extendedOccupancy = occupancies[2];
    if (mEnPassentSquare != 0) {
      extendedOccupancy |= mEnPassentSquare;
    }
    attack = attack::getPawnAttacks(square, color, extendedOccupancy);
  }
  return attack;
}

uint64_t Position::getPseudoLegalMoves(int square, int type, Color color) {
  return (attackGeneration(square, type, color) & ~occupancies[color]);
}

void Position::getCaptures(Color color, MoveList& moveList) {
  Color enemy = (color == WHITE) ? BLACK : WHITE;

  // 1. Define the target mask (Enemy pieces + En Passant square)
  // We ignore the enemy King for captures (it's illegal to capture the King),
  // but we can filter that out inside the loop or just assume pseudo-legal generation
  // handles it if your engine logic requires it.
  // Standard practice: Don't generate king captures.
  uint64_t enemies = occupancies[enemy];
  uint64_t captureMask = enemies;

  if (mEnPassentSquare != 0) {
    captureMask |= mEnPassentSquare;
  }

  // --- PAWN CAPTURES (Bitwise Optimization) ---
  // Instead of iterating every pawn, we shift the entire bitboard.
  uint64_t pawns = pieces[color][PAWN];
  uint64_t pawnAttacks = 0ULL;

  if (color == WHITE) {
    // Capture North-East (<< 9) - Exclude H file to avoid wrapping
    uint64_t ne = (pawns << 9) & NOT_A_FILE;
    // Capture North-West (<< 7) - Exclude A file
    uint64_t nw = (pawns << 7) & NOT_H_FILE;

    pawnAttacks = (ne | nw) & captureMask;

    while (pawnAttacks) {
      int to = __builtin_ctzll(pawnAttacks);

      // Determine which pawn moved.
      // If we came from NE (to-9), check if a pawn was there.
      // Note: It's possible for a square to be attacked by two pawns.
      // We must check valid sources.

      int from_ne = to - 9;
      int from_nw = to - 7;

      // Check potential source squares
      if (to >= 9 && (pawns & (1ULL << from_ne)) && (from_ne % 8 != 7)) {
        addPawnCaptureMove(from_ne, to, enemies, moveList);
      }
      // We use 'else if' or just separate checks.
      // Separate checks are needed if two pawns capture to the same square.
      if (to >= 7 && (pawns & (1ULL << from_nw)) && (from_nw % 8 != 0)) {
        addPawnCaptureMove(from_nw, to, enemies, moveList);
      }

      pawnAttacks &= (pawnAttacks - 1);  // Clear LS1B
    }
  } else {  // BLACK
    // Capture South-East (>> 7) - Exclude A file
    uint64_t se = (pawns >> 7) & NOT_A_FILE;
    // Capture South-West (>> 9) - Exclude H file
    uint64_t sw = (pawns >> 9) & NOT_H_FILE;

    pawnAttacks = (se | sw) & captureMask;

    while (pawnAttacks) {
      int to = __builtin_ctzll(pawnAttacks);
      int from_se = to + 7;
      int from_sw = to + 9;

      if (from_se < 64 && (pawns & (1ULL << from_se)) && (from_se % 8 != 0)) {
        addPawnCaptureMove(from_se, to, enemies, moveList);
      }
      if (from_sw < 64 && (pawns & (1ULL << from_sw)) && (from_sw % 8 != 7)) {
        addPawnCaptureMove(from_sw, to, enemies, moveList);
      }
      pawnAttacks &= (pawnAttacks - 1);
    }
  }

  // --- OTHER PIECES (Standard Loop but pre-masked) ---
  // Iterate from KNIGHT (1) to KING (5)
  for (int type = KNIGHT; type <= KING; type++) {
    uint64_t pieceBB = pieces[color][type];

    while (pieceBB) {
      int from = __builtin_ctzll(pieceBB);

      // OPTIMIZATION: Get attacks and IMMEDIATELY mask with captureMask
      // This prevents generating quiet moves and then filtering them later.
      uint64_t attacks = attackGeneration(from, type, color) & enemies;

      while (attacks) {
        int to = __builtin_ctzll(attacks);

        int victim = board[to];
        // Note: King capture check should theoretically not be needed if move gen is pseudo-legal
        // and previous moves were legal, but we keep safety if needed.
        if (victim != KING) {
          int score = 0;
          if (victim != NOPIECE) {
            static const int victimScores[] = {100, 300, 310, 500, 900, 20000};
            static const int aggressorScores[] = {100, 300, 310, 500, 900, 20000};
            score = victimScores[victim] - (aggressorScores[type] / 10) + 10000;
          }
          moveList.add(Move(from, to, NOPIECE), score);
        }

        attacks &= (attacks - 1);
      }

      pieceBB &= (pieceBB - 1);
    }
  }
}

// 3. Helper function for adding pawn moves (handling promotions/En Passant)
// Add this as a private helper method in your Position class or inline it.
inline void Position::addPawnCaptureMove(int from, int to, uint64_t enemies, MoveList& moveList) {
  int victim = board[to];
  int score = 0;

  // En Passant Case
  if (victim == NOPIECE) {  // Must be EP if we are here
    score = 105 + 10000;    // Value of pawn capture
    moveList.add(Move(from, to, NOPIECE), score);
    return;
  }

  // Normal Capture Score
  static const int victimScores[] = {100, 300, 310, 500, 900, 20000};
  score = victimScores[victim] - 10 + 10000;  // Pawn value ~100/10

  // Promotion Check
  // If target is Rank 0 or Rank 7
  if (to >= 56 || to <= 7) {
    score += 9000;  // Promotion bonus
    moveList.add(Move(from, to, QUEEN), score);
    moveList.add(Move(from, to, KNIGHT), score);
    moveList.add(Move(from, to, ROOK), score);
    moveList.add(Move(from, to, BISHOP), score);
  } else {
    moveList.add(Move(from, to, NOPIECE), score);
  }
}

bool Position::isSquareAttacked(int square, Color sideAttacking) {
  Color defendingSide = (sideAttacking == WHITE) ? BLACK : WHITE;
  if (attack::pawnAttacks[defendingSide][square] & pieces[sideAttacking][PAWN]) {
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

// Full castling mask array
// Indices: 0=a1, 7=h1, 56=a8, 63=h8
static const int castling_rights[64] = {
    // Rank 1 (White)
    ~WHITE_OOO, -1, -1, -1, ~(WHITE_OO | WHITE_OOO), -1, -1, ~WHITE_OO,
    // Rank 2-7 (Empty - no castling rights change)
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    // Rank 8 (Black)
    ~BLACK_OOO, -1, -1, -1, ~(BLACK_OO | BLACK_OOO), -1, -1, ~BLACK_OO};

void Position::doMove(Move m) {
  StateInfo state;
  state.castle = mCastleRight;
  state.epSquare = mEnPassentSquare;
  state.halfMove = mHalfMove;
  state.psqtScore = posEval.positionScore;
  state.zobristKey = mHash;
  state.movedPiece = board[m.from];
  state.capturedPiece = NOPIECE;

  Color enemy = (mSideToMove == WHITE) ? BLACK : WHITE;

  // Pre-compute masks
  uint64_t fromMask = 1ULL << m.from;
  uint64_t toMask = 1ULL << m.to;
  uint64_t moveMask = fromMask | toMask;

  // Update hash - remove old state contributions
  if (mEnPassentSquare) {
    mHash ^= Zobrist::enPassantKeys[__builtin_ctzll(mEnPassentSquare)];
  }
  mHash ^= Zobrist::castleKeys[mCastleRight];
  mHash ^= Zobrist::sideKey;
  mHash ^= Zobrist::pieceKeys[mSideToMove][state.movedPiece][m.from];

  // Update PSQT score - remove piece from source
  posEval.positionScore -= getPieceValue(state.movedPiece, m.from, mSideToMove);

  // Handle capture
  if (board[m.to] != NOPIECE) {
    state.capturedPiece = board[m.to];
    pieces[enemy][state.capturedPiece] ^= toMask;
    occupancies[enemy] ^= toMask;
    occupancies[2] ^= toMask;
    posEval.positionScore -= getPieceValue(state.capturedPiece, m.to, enemy);
    mHash ^= Zobrist::pieceKeys[enemy][state.capturedPiece][m.to];
    mHalfMove = 0;
  } else if (state.movedPiece == PAWN) {
    mHalfMove = 0;
  } else {
    mHalfMove++;
  }

  // Handle promotion
  if (m.promotion != NOPIECE) {
    pieces[mSideToMove][PAWN] ^= fromMask;
    pieces[mSideToMove][m.promotion] ^= toMask;
    occupancies[mSideToMove] ^= moveMask;
    occupancies[2] ^= moveMask;

    board[m.from] = NOPIECE;
    board[m.to] = m.promotion;

    posEval.positionScore += getPieceValue(m.promotion, m.to, mSideToMove);
    mHash ^= Zobrist::pieceKeys[mSideToMove][m.promotion][m.to];
    mEnPassentSquare = 0;
  }
  // Handle normal moves
  else {
    pieces[mSideToMove][state.movedPiece] ^= moveMask;
    occupancies[mSideToMove] ^= moveMask;
    occupancies[2] ^= moveMask;

    board[m.from] = NOPIECE;
    board[m.to] = state.movedPiece;

    posEval.positionScore += getPieceValue(state.movedPiece, m.to, mSideToMove);
    mHash ^= Zobrist::pieceKeys[mSideToMove][state.movedPiece][m.to];

    // Handle castling
    if (state.movedPiece == KING) {
      int castleDelta = (int)m.to - (int)m.from;

      if (abs(castleDelta) == 2) {  // Kingside
        int rookIdx = (castleDelta == 2) ? (m.to + 1) : (m.to - 2);
        int rookDest = (castleDelta == 2) ? (m.to - 1) : (m.to + 1);

        uint64_t rookMask = (1ULL << rookIdx) | (1ULL << rookDest);

        pieces[mSideToMove][ROOK] ^= rookMask;
        occupancies[mSideToMove] ^= rookMask;
        occupancies[2] ^= rookMask;

        board[rookIdx] = NOPIECE;
        board[rookDest] = ROOK;

        posEval.positionScore -= getPieceValue(ROOK, rookIdx, mSideToMove);
        posEval.positionScore += getPieceValue(ROOK, rookDest, mSideToMove);
        mHash ^= Zobrist::pieceKeys[mSideToMove][ROOK][rookIdx];
        mHash ^= Zobrist::pieceKeys[mSideToMove][ROOK][rookDest];
      }
      mEnPassentSquare = 0;
    }
    // Handle pawn moves
    else if (state.movedPiece == PAWN) {
      // Handle en passant capture
      if (toMask == mEnPassentSquare && mEnPassentSquare != 0) {
        int captureOffset = (mSideToMove == WHITE) ? -8 : 8;
        int captureSq = m.to + captureOffset;
        uint64_t captureMask = 1ULL << captureSq;

        state.capturedPiece = PAWN;
        pieces[enemy][PAWN] ^= captureMask;
        occupancies[enemy] ^= captureMask;
        occupancies[2] ^= captureMask;
        board[captureSq] = NOPIECE;

        posEval.positionScore -= getPieceValue(PAWN, captureSq, enemy);
        mHash ^= Zobrist::pieceKeys[enemy][PAWN][captureSq];
      }

      // Set new en passant square for double pawn push
      if (abs((int)m.to - (int)m.from) == 16) {
        int epSq = (m.from + m.to) / 2;
        mEnPassentSquare = 1ULL << epSq;
      } else {
        mEnPassentSquare = 0;
      }
    } else {
      mEnPassentSquare = 0;
    }
  }

  // Update castle rights
  mCastleRight &= castling_rights[m.from];
  mCastleRight &= castling_rights[m.to];

  // Update hash - add new state contributions
  if (mEnPassentSquare) {
    mHash ^= Zobrist::enPassantKeys[__builtin_ctzll(mEnPassentSquare)];
  }
  mHash ^= Zobrist::castleKeys[mCastleRight];

  // Switch side
  mSideToMove = enemy;

  history[gamePly] = state;

  history[gamePly].evalCache.invalidate();

  gamePly++;
}

void Position::undoMove(Move m) {
  gamePly--;
  // Restore state from history
  const StateInfo& state = history[gamePly];

  mCastleRight = state.castle;
  mEnPassentSquare = state.epSquare;
  mHalfMove = state.halfMove;
  mHash = state.zobristKey;
  posEval.positionScore = state.psqtScore;

  // Switch side back
  mSideToMove = (mSideToMove == WHITE) ? BLACK : WHITE;
  Color enemy = (mSideToMove == WHITE) ? BLACK : WHITE;

  // Pre-compute masks
  uint64_t fromMask = 1ULL << m.from;
  uint64_t toMask = 1ULL << m.to;
  uint64_t moveMask = fromMask | toMask;

  // Handle promotion undo
  if (m.promotion != NOPIECE) {
    pieces[mSideToMove][m.promotion] ^= toMask;
    pieces[mSideToMove][PAWN] ^= fromMask;
    occupancies[mSideToMove] ^= moveMask;
    occupancies[2] ^= moveMask;

    board[m.to] = NOPIECE;
    board[m.from] = PAWN;
  }
  // Handle normal move undo
  else {
    pieces[mSideToMove][state.movedPiece] ^= moveMask;
    occupancies[mSideToMove] ^= moveMask;
    occupancies[2] ^= moveMask;

    board[m.to] = NOPIECE;
    board[m.from] = state.movedPiece;

    // Handle castling undo
    if (state.movedPiece == KING) {
      int castleDelta = (int)m.to - (int)m.from;

      if (castleDelta == 2) {  // Kingside
        uint64_t rookMoveMask = (1ULL << (m.to + 1)) | (1ULL << (m.to - 1));
        pieces[mSideToMove][ROOK] ^= rookMoveMask;
        occupancies[mSideToMove] ^= rookMoveMask;
        occupancies[2] ^= rookMoveMask;

        board[m.to - 1] = NOPIECE;
        board[m.to + 1] = ROOK;
      } else if (castleDelta == -2) {  // Queenside
        uint64_t rookMoveMask = (1ULL << (m.to - 2)) | (1ULL << (m.to + 1));
        pieces[mSideToMove][ROOK] ^= rookMoveMask;
        occupancies[mSideToMove] ^= rookMoveMask;
        occupancies[2] ^= rookMoveMask;

        board[m.to + 1] = NOPIECE;
        board[m.to - 2] = ROOK;
      }
    }
  }

  // Restore captured piece
  if (state.capturedPiece != NOPIECE) {
    // Handle en passant capture undo
    if (state.movedPiece == PAWN && toMask == state.epSquare) {
      int captureOffset = (mSideToMove == WHITE) ? -8 : 8;
      int captureSq = m.to + captureOffset;
      uint64_t captureMask = 1ULL << captureSq;

      pieces[enemy][PAWN] ^= captureMask;
      occupancies[enemy] ^= captureMask;
      occupancies[2] ^= captureMask;
      board[captureSq] = PAWN;
    }
    // Handle normal capture undo
    else {
      pieces[enemy][state.capturedPiece] ^= toMask;
      occupancies[enemy] ^= toMask;
      occupancies[2] ^= toMask;
      board[m.to] = state.capturedPiece;
    }
  }
}

void Position::getMoves(Color color, MoveList& moveList) {
  Color enemy = (color == WHITE) ? BLACK : WHITE;
  uint64_t enemyKing = pieces[enemy][KING];
  if (enemyKing == 0) {
    std::cout << "CRITICAL ERROR: Enemy King missing for color " << enemy << std::endl;
  }

  for (int i = 0; i < 6; i++) {
    uint64_t piece = pieces[color][i];
    while (piece) {
      int sourceSquare = __builtin_ctzll(piece);
      uint64_t validTargets = getPseudoLegalMoves(sourceSquare, i, color);
      validTargets &= ~enemyKing;

      while (validTargets) {
        int targetSquare = __builtin_ctzll(validTargets);
        int victim = board[targetSquare];
        int aggressor = i;
        int score = 0;

        if (victim != NOPIECE) {
          static const int victimScores[] = {100, 300, 310, 500, 900, 0};
          score = victimScores[victim] - victimScores[aggressor] + 10000;
        }

        if (i == PAWN && (targetSquare / 8) == ((color == WHITE) ? 7 : 0)) {
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
    if ((mCastleRight & WHITE_OO) && !(occupancies[2] & ((1ULL << 5) | (1ULL << 6)))) {
      if (!isSquareAttacked(4, BLACK) && !isSquareAttacked(5, BLACK) &&
          !isSquareAttacked(6, BLACK)) {
        moveList.add(Move(4, 6, NOPIECE), 0);  // e1g1
      }
    }

    if ((mCastleRight & WHITE_OOO) &&
        !(occupancies[2] & ((1ULL << 1) | (1ULL << 2) | (1ULL << 3)))) {
      if (!isSquareAttacked(4, BLACK) && !isSquareAttacked(3, BLACK) &&
          !isSquareAttacked(2, BLACK)) {
        moveList.add(Move(4, 2, NOPIECE), 0);  // e1c1
      }
    }
  } else {
    if ((mCastleRight & BLACK_OO) && !(occupancies[2] & ((1ULL << 61) | (1ULL << 62)))) {
      if (!isSquareAttacked(60, WHITE) && !isSquareAttacked(61, WHITE) &&
          !isSquareAttacked(62, WHITE)) {
        moveList.add(Move(60, 62, NOPIECE), 0);  // e8g8
      }
    }

    if ((mCastleRight & BLACK_OOO) &&
        !(occupancies[2] & ((1ULL << 57) | (1ULL << 58) | (1ULL << 59)))) {
      if (!isSquareAttacked(60, WHITE) && !isSquareAttacked(59, WHITE) &&
          !isSquareAttacked(58, WHITE)) {
        moveList.add(Move(60, 58, NOPIECE), 0);  // e8c8
      }
    }
  }
}

void Position::printBoard() {
  std::cout << "         board" << std::endl;
  std::cout << "  +-----------------+" << std::endl;
  for (int rank = 7; rank >= 0; rank--) {
    std::cout << rank + 1 << " | ";
    for (int file = 0; file < 8; file++) {
      int square = rank * 8 + file;
      if ((occupancies[WHITE] >> square) & 1) {
        if (board[square] == PAWN) {
          std::cout << "P ";
        } else if (board[square] == ROOK) {
          std::cout << "R ";
        } else if (board[square] == BISHOP) {
          std::cout << "B ";
        } else if (board[square] == QUEEN) {
          std::cout << "Q ";
        } else if (board[square] == KNIGHT) {
          std::cout << "N ";
        } else if (board[square] == KING) {
          std::cout << "K ";
        }
      } else if ((occupancies[BLACK] >> square) & 1) {
        if (board[square] == PAWN) {
          std::cout << "p ";
        } else if (board[square] == ROOK) {
          std::cout << "r ";
        } else if (board[square] == BISHOP) {
          std::cout << "b ";
        } else if (board[square] == QUEEN) {
          std::cout << "q ";
        } else if (board[square] == KNIGHT) {
          std::cout << "n ";
        } else if (board[square] == KING) {
          std::cout << "k ";
        }
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
  Zobrist::init();
  initPST();
  gamePly = 0;

  for (int c = 0; c < 2; c++) {
    for (int p = 0; p < 6; p++) {
      pieces[c][p] = 0ULL;
    }
  }
  occupancies[0] = 0ULL;
  occupancies[1] = 0ULL;
  occupancies[2] = 0ULL;
  mSideToMove = WHITE;
  mCastleRight = 0;
  mEnPassentSquare = 0;
  mHash = 0ULL;
}
Position::~Position() {}
