#include "attack.hpp"

#include <cstdint>

#include "magicBitboards.hpp"
#include "types.hpp"

void attack::computeKnightAttacks() {
  uint64_t mask = 0ULL;
  for (int i = 0; i < 64; i++) {
    int rank = i / 8;
    int file = i % 8;
    // bottom left
    if (file > 0 && rank > 1) {
      mask |= 1ULL << ((rank - 2) * 8 + (file - 1));
    }
    // left bottom
    if (file > 1 && rank > 0) {
      mask |= 1ULL << ((rank - 1) * 8 + (file - 2));
    }
    // left top
    if (file > 1 && rank < 7) {
      mask |= 1ULL << ((rank + 1) * 8 + (file - 2));
    }
    // top left
    if (file > 0 && rank < 6) {
      mask |= 1ULL << ((rank + 2) * 8 + (file - 1));
    }
    // top right
    if (file < 7 && rank < 6) {
      mask |= 1ULL << ((rank + 2) * 8 + (file + 1));
    }
    // right top
    if (file < 6 && rank < 7) {
      mask |= 1ULL << ((rank + 1) * 8 + (file + 2));
    }
    // right bottom
    if (file < 6 && rank > 0) {
      mask |= 1ULL << ((rank - 1) * 8 + (file + 2));
    }
    // bottom right
    if (file < 7 && rank > 1) {
      mask |= 1ULL << ((rank - 2) * 8 + (file + 1));
    }
    knightAttacks[i] = mask;

    mask = 0ULL;
  }
}

void attack::computeKingAttacks() {
  uint64_t mask = 0ULL;
  for (int i = 0; i < 64; i++) {
    int rank = i / 8;
    int file = i % 8;
    // bottom left
    if (file > 0 && rank > 0) {
      mask |= 1ULL << ((rank - 1) * 8 + (file - 1));
    }
    // left
    if (file > 0) {
      mask |= 1ULL << (rank * 8 + (file - 1));
    }
    // top left
    if (file > 0 && rank < 7) {
      mask |= 1ULL << ((rank + 1) * 8 + (file - 1));
    }
    // top
    if (rank < 7) {
      mask |= 1ULL << ((rank + 1) * 8 + file);
    }
    // top right
    if (file < 7 && rank < 7) {
      mask |= 1ULL << ((rank + 1) * 8 + (file + 1));
    }
    // right
    if (file < 7) {
      mask |= 1ULL << (rank * 8 + (file + 1));
    }
    // bottom right
    if (file < 7 && rank > 0) {
      mask |= 1ULL << ((rank - 1) * 8 + (file + 1));
    }
    // bottom
    if (rank > 0) {
      mask |= 1ULL << ((rank - 1) * 8 + file);
    }
    kingAttacks[i] = mask;

    mask = 0ULL;
  }
}

void attack::initRays() {
  for (int sq = 0; sq < 64; sq++) {
    int r = sq / 8;
    int f = sq % 8;

    for (int i = 1; i < 8; i++) {
      if (r + i > 7) break;

      int target = (r + i) * 8 + f;
      Rays[0][sq] |= (1ULL << target);
    }

    for (int i = 1; i < 8; i++) {
      if (r - i < 0) break;

      int target = (r - i) * 8 + f;
      Rays[1][sq] |= (1ULL << target);
    }

    for (int i = 1; i < 8; i++) {
      if (f + i > 7) break;

      int target = r * 8 + (f + i);
      Rays[2][sq] |= (1ULL << target);
    }

    for (int i = 1; i < 8; i++) {
      if (f - i < 0) break;

      int target = r * 8 + (f - i);
      Rays[3][sq] |= (1ULL << target);
    }

    for (int i = 1; i < 8; i++) {
      if (r + i > 7 || f + i > 7) break;

      int target = (r + i) * 8 + (f + i);
      Rays[4][sq] |= (1ULL << target);
    }

    for (int i = 1; i < 8; i++) {
      if (r + i > 7 || f - i < 0) break;

      int target = (r + i) * 8 + (f - i);
      Rays[5][sq] |= (1ULL << target);
    }

    for (int i = 1; i < 8; i++) {
      if (r - i < 0 || f + i > 7) break;

      int target = (r - i) * 8 + (f + i);
      Rays[6][sq] |= (1ULL << target);
    }

    for (int i = 1; i < 8; i++) {
      if (r - i < 0 || f - i < 0) break;

      int target = (r - i) * 8 + (f - i);
      Rays[7][sq] |= (1ULL << target);
    }
  }
}

void attack::initPawnAttacks() {
  for (int sq = 0; sq < 64; sq++) {
    if (sq % 8 != 0 && (sq + 7) < 64) pawnAttacks[WHITE][sq] |= (1ULL << (sq + 7));
    if (sq % 8 != 7 && (sq + 9) < 64) pawnAttacks[WHITE][sq] |= (1ULL << (sq + 9));
  }

  for (int sq = 0; sq < 64; sq++) {
    if (sq % 8 != 7 && (sq - 7) >= 0) pawnAttacks[BLACK][sq] |= (1ULL << (sq - 7));
    if (sq % 8 != 0 && (sq - 9) >= 0) pawnAttacks[BLACK][sq] |= (1ULL << (sq - 9));
  }
}

uint64_t attack::getRookAttacks(int square, uint64_t occupancy) {
  return MagicBitboards::getRookAttacks(square, occupancy);
}

uint64_t attack::getBishopAttacks(int square, uint64_t occupancy) {
  return MagicBitboards::getBishopAttacks(square, occupancy);
}

uint64_t attack::getQueenAttacks(int square, uint64_t occupancy) {
  return MagicBitboards::getQueenAttacks(square, occupancy);
}

uint64_t attack::getPawnAttacks(int square, Color color, uint64_t occupancy) {
  uint64_t attacks = 0ULL;

  int direction = (color == WHITE) ? 8 : -8;
  int startRank = (color == WHITE) ? 1 : 6;

  int target = square + direction;

  if (!(occupancy & (1ULL << target))) {
    attacks |= (1ULL << target);
    if ((square / 8) == startRank && !(occupancy & (1ULL << (square + (direction * 2))))) {
      attacks |= (1ULL << (square + (direction * 2)));
    }
  }

  uint64_t attackingSquares = pawnAttacks[color][square];

  attackingSquares &= occupancy;

  while (attackingSquares) {
    int capture_sq = __builtin_ctzll(attackingSquares);
    attacks |= (1ULL << capture_sq);
    attackingSquares &= (attackingSquares - 1);
  }

  return attacks;
}

namespace attack {
uint64_t knightAttacks[64];
uint64_t kingAttacks[64];
uint64_t Rays[8][64];
uint64_t pawnAttacks[2][64];
}  // namespace attack

void attack::init() {
  computeKnightAttacks();
  computeKingAttacks();
  initRays();
  initPawnAttacks();
  MagicBitboards::init();
}
