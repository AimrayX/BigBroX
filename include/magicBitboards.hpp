#ifndef MAGICBITBOARDS_H
#define MAGICBITBOARDS_H

#include <cstdint>

// Define Bitboard as a 64-bit unsigned integer
typedef uint64_t Bitboard;

struct Magic {
    Bitboard* attacks;  // Pointer to the attack table for this square
    Bitboard  mask;     // Mask of relevant blockers
    Bitboard  magic;    // The magic number
    int       shift;    // Shift amount
};

void init_magic_bitboards();

Bitboard get_rook_attacks(int square, Bitboard occupancy);
Bitboard get_bishop_attacks(int square, Bitboard occupancy);

#endif
