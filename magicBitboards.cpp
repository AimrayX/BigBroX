#include "magicBitboards.hpp"

uint64_t MagicBitboards::rookMasks[64];
uint64_t MagicBitboards::bishopMasks[64];
uint64_t MagicBitboards::rookAttacks[64][4096];
uint64_t MagicBitboards::bishopAttacks[64][512];

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

// Generate rook attacks the slow way (for initialization)
uint64_t MagicBitboards::generateRookAttacksSlow(int square, uint64_t occupancy) {
  uint64_t attacks = 0ULL;
  int rank = square / 8;
  int file = square % 8;

  // North
  for (int r = rank + 1; r <= 7; r++) {
    int sq = r * 8 + file;
    attacks |= (1ULL << sq);
    if (occupancy & (1ULL << sq)) break;
  }

  // South
  for (int r = rank - 1; r >= 0; r--) {
    int sq = r * 8 + file;
    attacks |= (1ULL << sq);
    if (occupancy & (1ULL << sq)) break;
  }

  // East
  for (int f = file + 1; f <= 7; f++) {
    int sq = rank * 8 + f;
    attacks |= (1ULL << sq);
    if (occupancy & (1ULL << sq)) break;
  }

  // West
  for (int f = file - 1; f >= 0; f--) {
    int sq = rank * 8 + f;
    attacks |= (1ULL << sq);
    if (occupancy & (1ULL << sq)) break;
  }

  return attacks;
}

// Generate bishop attacks the slow way (for initialization)
uint64_t MagicBitboards::generateBishopAttacksSlow(int square, uint64_t occupancy) {
  uint64_t attacks = 0ULL;
  int rank = square / 8;
  int file = square % 8;

  // NE
  for (int r = rank + 1, f = file + 1; r <= 7 && f <= 7; r++, f++) {
    int sq = r * 8 + f;
    attacks |= (1ULL << sq);
    if (occupancy & (1ULL << sq)) break;
  }

  // NW
  for (int r = rank + 1, f = file - 1; r <= 7 && f >= 0; r++, f--) {
    int sq = r * 8 + f;
    attacks |= (1ULL << sq);
    if (occupancy & (1ULL << sq)) break;
  }

  // SE
  for (int r = rank - 1, f = file + 1; r >= 0 && f <= 7; r--, f++) {
    int sq = r * 8 + f;
    attacks |= (1ULL << sq);
    if (occupancy & (1ULL << sq)) break;
  }

  // SW
  for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--) {
    int sq = r * 8 + f;
    attacks |= (1ULL << sq);
    if (occupancy & (1ULL << sq)) break;
  }

  return attacks;
}

// Generate relevant occupancy mask (exclude edge squares)
uint64_t MagicBitboards::generateRookMask(int square) {
  uint64_t mask = 0ULL;
  int rank = square / 8;
  int file = square % 8;

  // North (exclude rank 7)
  for (int r = rank + 1; r <= 6; r++) {
    mask |= (1ULL << (r * 8 + file));
  }

  // South (exclude rank 0)
  for (int r = rank - 1; r >= 1; r--) {
    mask |= (1ULL << (r * 8 + file));
  }

  // East (exclude file 7)
  for (int f = file + 1; f <= 6; f++) {
    mask |= (1ULL << (rank * 8 + f));
  }

  // West (exclude file 0)
  for (int f = file - 1; f >= 1; f--) {
    mask |= (1ULL << (rank * 8 + f));
  }

  return mask;
}

uint64_t MagicBitboards::generateBishopMask(int square) {
  uint64_t mask = 0ULL;
  int rank = square / 8;
  int file = square % 8;

  // NE (exclude edges)
  for (int r = rank + 1, f = file + 1; r <= 6 && f <= 6; r++, f++) {
    mask |= (1ULL << (r * 8 + f));
  }

  // NW
  for (int r = rank + 1, f = file - 1; r <= 6 && f >= 1; r++, f--) {
    mask |= (1ULL << (r * 8 + f));
  }

  // SE
  for (int r = rank - 1, f = file + 1; r >= 1 && f <= 6; r--, f++) {
    mask |= (1ULL << (r * 8 + f));
  }

  // SW
  for (int r = rank - 1, f = file - 1; r >= 1 && f >= 1; r--, f--) {
    mask |= (1ULL << (r * 8 + f));
  }

  return mask;
}

// Enumerate all occupancy variations for a mask
void MagicBitboards::enumerateOccupancies(uint64_t mask, int bits, uint64_t* occupancies) {
  int n = 1 << bits;

  for (int i = 0; i < n; i++) {
    uint64_t occ = 0ULL;
    uint64_t temp = mask;
    int idx = 0;

    while (temp) {
      int sq = __builtin_ctzll(temp);
      if (i & (1 << idx)) {
        occ |= (1ULL << sq);
      }
      temp &= (temp - 1);
      idx++;
    }

    occupancies[i] = occ;
  }
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void MagicBitboards::init() {
  // Generate masks
  for (int sq = 0; sq < 64; sq++) {
    rookMasks[sq] = generateRookMask(sq);
    bishopMasks[sq] = generateBishopMask(sq);
  }

  // Initialize rook attack tables
  for (int sq = 0; sq < 64; sq++) {
    uint64_t mask = rookMasks[sq];
    int bits = __builtin_popcountll(mask);
    int n = 1 << bits;

    uint64_t occupancies[4096];
    enumerateOccupancies(mask, bits, occupancies);

    for (int i = 0; i < n; i++) {
      uint64_t occ = occupancies[i];
      uint64_t attacks = generateRookAttacksSlow(sq, occ);

      // Hash the occupancy
      uint64_t index = (occ * rookMagics[sq]) >> rookShifts[sq];
      rookAttacks[sq][index] = attacks;
    }
  }

  // Initialize bishop attack tables
  for (int sq = 0; sq < 64; sq++) {
    uint64_t mask = bishopMasks[sq];
    int bits = __builtin_popcountll(mask);
    int n = 1 << bits;

    uint64_t occupancies[512];
    enumerateOccupancies(mask, bits, occupancies);

    for (int i = 0; i < n; i++) {
      uint64_t occ = occupancies[i];
      uint64_t attacks = generateBishopAttacksSlow(sq, occ);

      // Hash the occupancy
      uint64_t index = (occ * bishopMagics[sq]) >> bishopShifts[sq];
      bishopAttacks[sq][index] = attacks;
    }
  }
}

