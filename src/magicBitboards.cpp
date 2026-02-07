#include "../include/magicBitboards.hpp"

#include <cstring>

Magic rookMagics[64];
Magic bishopMagics[64];

// Huge array to store all precomputed attacks (~2.3 MB)
// Rooks need 4096 per square (12 bits), Bishops need fewer.
Bitboard rookAttackTable[64 * 4096];
Bitboard bishopAttackTable[64 * 512];

// 1. Random Number Generator for finding Magics
uint64_t random_u64() {
  static uint64_t seed = 1804289383;
  // XORShift algorithm for fast pseudo-random numbers
  seed ^= seed << 13;
  seed ^= seed >> 7;
  seed ^= seed << 17;
  return seed;
}

// Generate a candidate magic number (sparse, few 1s)
uint64_t random_u64_fewbits() { return random_u64() & random_u64() & random_u64(); }

// 2. Slow/Standard Attack Generation (used only for validation)
Bitboard get_rook_mask(int sq) {
  Bitboard attacks = 0;
  int r = sq / 8, f = sq % 8;
  for (int r2 = r + 1; r2 <= 6; r2++) attacks |= (1ULL << (r2 * 8 + f));
  for (int r2 = r - 1; r2 >= 1; r2--) attacks |= (1ULL << (r2 * 8 + f));
  for (int f2 = f + 1; f2 <= 6; f2++) attacks |= (1ULL << (r * 8 + f2));
  for (int f2 = f - 1; f2 >= 1; f2--) attacks |= (1ULL << (r * 8 + f2));
  return attacks;
}

Bitboard get_bishop_mask(int sq) {
  Bitboard attacks = 0;
  int r = sq / 8, f = sq % 8;
  for (int r2 = r + 1, f2 = f + 1; r2 <= 6 && f2 <= 6; r2++, f2++)
    attacks |= (1ULL << (r2 * 8 + f2));
  for (int r2 = r + 1, f2 = f - 1; r2 <= 6 && f2 >= 1; r2++, f2--)
    attacks |= (1ULL << (r2 * 8 + f2));
  for (int r2 = r - 1, f2 = f + 1; r2 >= 1 && f2 <= 6; r2--, f2++)
    attacks |= (1ULL << (r2 * 8 + f2));
  for (int r2 = r - 1, f2 = f - 1; r2 >= 1 && f2 >= 1; r2--, f2--)
    attacks |= (1ULL << (r2 * 8 + f2));
  return attacks;
}

Bitboard rook_attacks_slow(int sq, Bitboard block) {
  Bitboard attacks = 0;
  int r = sq / 8, f = sq % 8;
  for (int r2 = r + 1; r2 <= 7; r2++) {
    attacks |= (1ULL << (r2 * 8 + f));
    if (block & (1ULL << (r2 * 8 + f))) break;
  }
  for (int r2 = r - 1; r2 >= 0; r2--) {
    attacks |= (1ULL << (r2 * 8 + f));
    if (block & (1ULL << (r2 * 8 + f))) break;
  }
  for (int f2 = f + 1; f2 <= 7; f2++) {
    attacks |= (1ULL << (r * 8 + f2));
    if (block & (1ULL << (r * 8 + f2))) break;
  }
  for (int f2 = f - 1; f2 >= 0; f2--) {
    attacks |= (1ULL << (r * 8 + f2));
    if (block & (1ULL << (r * 8 + f2))) break;
  }
  return attacks;
}

Bitboard bishop_attacks_slow(int sq, Bitboard block) {
  Bitboard attacks = 0;
  int r = sq / 8, f = sq % 8;
  for (int r2 = r + 1, f2 = f + 1; r2 <= 7 && f2 <= 7; r2++, f2++) {
    attacks |= (1ULL << (r2 * 8 + f2));
    if (block & (1ULL << (r2 * 8 + f2))) break;
  }
  for (int r2 = r + 1, f2 = f - 1; r2 <= 7 && f2 >= 0; r2++, f2--) {
    attacks |= (1ULL << (r2 * 8 + f2));
    if (block & (1ULL << (r2 * 8 + f2))) break;
  }
  for (int r2 = r - 1, f2 = f + 1; r2 >= 0 && f2 <= 7; r2--, f2++) {
    attacks |= (1ULL << (r2 * 8 + f2));
    if (block & (1ULL << (r2 * 8 + f2))) break;
  }
  for (int r2 = r - 1, f2 = f - 1; r2 >= 0 && f2 >= 0; r2--, f2--) {
    attacks |= (1ULL << (r2 * 8 + f2));
    if (block & (1ULL << (r2 * 8 + f2))) break;
  }
  return attacks;
}

int count_1s(Bitboard b) {
  int r = 0;
  while (b) {
    r++;
    b &= b - 1;
  }
  return r;
}

// This performs the "Magic": (Blockers & Mask) * Magic >> Shift
int transform(Bitboard b, uint64_t magic, int bits) { return (int)((b * magic) >> (64 - bits)); }

// This function loops until it finds a magic number that works for a specific square
void find_magic(int sq, int m, int bishop) {
  Bitboard mask = bishop ? get_bishop_mask(sq) : get_rook_mask(sq);
  int blockBits = count_1s(mask);
  int var = 1 << blockBits;  // Number of blocker variations

  // Arrays to store test data
  Bitboard blockers[4096];
  Bitboard attacks[4096];
  Bitboard used[4096];

  // Precompute all blocker/attack pairs for this square
  for (int i = 0; i < var; i++) {
    blockers[i] = 0;
    Bitboard tempMask = mask;
    // Map index 'i' to a blocker bitboard
    for (int bit = 0; bit < blockBits; bit++) {
      int lsb = __builtin_ctzll(tempMask);  // Get least significant bit index
      tempMask &= tempMask - 1;             // Clear LSB
      if (i & (1 << bit)) blockers[i] |= (1ULL << lsb);
    }
    attacks[i] = bishop ? bishop_attacks_slow(sq, blockers[i]) : rook_attacks_slow(sq, blockers[i]);
  }

  // Trial and Error Loop
  for (int k = 0; k < 100000000; k++) {
    uint64_t magic = random_u64_fewbits();

    // This check is a heuristic to reject obviously bad magics quickly
    if (count_1s((mask * magic) & 0xFF00000000000000ULL) < 6) continue;

    // Verify the magic
    memset(used, 0, sizeof(used));
    bool fail = false;

    for (int i = 0; i < var; i++) {
      int index = transform(blockers[i], magic, blockBits);

      if (used[index] == 0) {
        used[index] = attacks[i];
      } else if (used[index] != attacks[i]) {
        fail = true;  // Collision! Two different attacks map to same index
        break;
      }
    }

    if (!fail) {
      // Found one! Save it.
      Magic& mag = bishop ? bishopMagics[sq] : rookMagics[sq];
      mag.mask = mask;
      mag.magic = magic;
      mag.shift = 64 - blockBits;

      // Point to the correct part of the global table
      if (bishop)
        mag.attacks = &bishopAttackTable[sq * 512];
      else
        mag.attacks = &rookAttackTable[sq * 4096];

      // Fill the actual table
      for (int i = 0; i < var; i++) {
        int index = transform(blockers[i], magic, blockBits);
        mag.attacks[index] = attacks[i];
      }
      return;
    }
  }
}

void init_magic_bitboards() {
  for (int i = 0; i < 64; i++) find_magic(i, 12, 0);  // Rooks
  for (int i = 0; i < 64; i++) find_magic(i, 9, 1);   // Bishops
}

Bitboard get_rook_attacks(int square, Bitboard occupancy) {
  Magic& m = rookMagics[square];
  occupancy &= m.mask;
  occupancy *= m.magic;
  occupancy >>= m.shift;
  return m.attacks[occupancy];
}

Bitboard get_bishop_attacks(int square, Bitboard occupancy) {
  Magic& m = bishopMagics[square];
  occupancy &= m.mask;
  occupancy *= m.magic;
  occupancy >>= m.shift;
  return m.attacks[occupancy];
}
