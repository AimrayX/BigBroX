#include "attack.hpp"

int mComputeKnightAttacks() {
    for (int i = 0; i < 64; i++)
    {
        
    }
    
    return 0;
}

int mComputeKingAttacks() {

    return 0;
}

uint64_t attack::mVertHorMask(uint64_t piece) {
    int rank = __builtin_ctzll(piece) / 8;
    int file = __builtin_ctzll(piece) % 8;

    uint64_t mask = 0ULL;
    for (int r = rank + 1; r < 7; r++) mask |= 1ULL << (r * 8 + file);
    for (int r = rank - 1; r > 0; r--) mask |= 1ULL << (r * 8 + file);
    for (int f = file + 1; f < 7; f++) mask |= 1ULL << (rank * 8 + f);
    for (int f = file - 1; f > 0; f--) mask |= 1ULL << (rank * 8 + f);

    return mask;
}

uint64_t attack::mDiagonalMask(uint64_t piece) {
    int rank = __builtin_ctzll(piece) / 8;
    int file = __builtin_ctzll(piece) % 8;

    uint64_t mask = 0ULL;
    for (int r = rank + 1, f = file + 1; r < 7 && f < 7; r++, f++) {
        mask |= 1ULL << (r * 8 + f);
    }
    for (int r = rank + 1, f = file - 1; r < 7 && f > 0; r++, f--) {
        mask |= 1ULL << (r * 8 + f);
    }
    for (int r = rank - 1, f = file + 1; r > 0 && f < 7; r--, f++) {
        mask |= 1ULL << (r * 8 + f);
    }
    for (int r = rank - 1, f = file - 1; r > 0 && f > 0; r--, f--) {
        mask |= 1ULL << (r * 8 + f);
    }

    return mask;
}

uint64_t knightAttacks[64];
uint64_t kingAttacks[64];

void attack::init() {

}

