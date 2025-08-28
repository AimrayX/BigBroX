#include "attack.hpp"

void attack::computeKnightAttacks() {
    uint64_t mask = 0ULL;
    for (int i = 0; i < 64; i++) {

        int rank = i / 8;
        int file = i % 8;
        //bottom left
        if (file > 0 && rank > 1) {
            mask |= 1ULL << ((rank - 2) * 8 + (file - 1));
        }
        //left bottom
        if (file > 1 && rank > 0) {
            mask |= 1ULL << ((rank - 1) * 8 + (file - 2));
        }
        //left top
        if (file > 1 && rank < 7) {
            mask |= 1ULL << ((rank + 1) * 8 + (file - 2));
        }
        //top left
        if (file > 0 && rank < 6) {
            mask |= 1ULL << ((rank + 2) * 8 + (file - 1));
        }
        //top right
        if (file < 7 && rank < 6) {
            mask |= 1ULL << ((rank + 2) * 8 + (file + 1));
        }
        //right top
        if (file < 6 && rank < 7) {
            mask |= 1ULL << ((rank + 1) * 8 + (file + 2));
        }
        //right bottom
        if (file < 6 && rank > 0) {
            mask |= 1ULL << ((rank - 1) * 8 + (file + 2));
        }
        //bottom right
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
        //bottom left
        if (file > 0 && rank > 0) {
            mask |= 1ULL << ((rank - 1) * 8 + (file - 1));
        }
        //left
        if (file > 0) {
            mask |= 1ULL << (rank * 8 + (file - 1));
        }
        //top left
        if (file > 0 && rank < 7) {
            mask |= 1ULL << ((rank + 1) * 8 + (file - 1));
        }
        //top
        if (rank < 7) {
            mask |= 1ULL << ((rank + 1) * 8 + file);
        }
        //top right
        if (file < 7 && rank < 7) {
            mask |= 1ULL << ((rank + 1) * 8 + (file + 1));
        }
        //right
        if (file < 7) {
            mask |= 1ULL << (rank * 8 + (file + 1));
        }
        //bottom right
        if (file < 7 && rank > 0) {
            mask |= 1ULL << ((rank - 1) * 8 + (file + 1));
        }
        //bottom
        if (rank > 0) {
            mask |= 1ULL << ((rank - 1) * 8 + file);
        }
        kingAttacks[i] = mask;

        mask = 0ULL;
    }
}

uint64_t attack::vertHorMask(int index) {
    int rank = index / 8;
    int file = 7 - (index % 8);

    uint64_t mask = 0ULL;
    for (int r = rank + 1; r < 7; r++) mask |= 1ULL << (r * 8 + file);
    for (int r = rank - 1; r > 0; r--) mask |= 1ULL << (r * 8 + file);
    for (int f = file + 1; f < 7; f++) mask |= 1ULL << (rank * 8 + f);
    for (int f = file - 1; f > 0; f--) mask |= 1ULL << (rank * 8 + f);

    return mask;
}

uint64_t attack::diagonalMask(int index) {
    int rank = index / 8;
    int file = 7 - (index % 8);

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

namespace attack {
    uint64_t knightAttacks[64];
    uint64_t kingAttacks[64];
}

void attack::init() {
    computeKnightAttacks();
    computeKingAttacks();
}

