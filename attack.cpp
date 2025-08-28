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

uint64_t attack::vertHorRay(int index, const uint64_t& whitePieces, const uint64_t& blackPieces) {
    int rank = index / 8;
    int file = index % 8;

    uint64_t bitOnBoard = 1ULL << index;
    uint64_t color = whitePieces & bitOnBoard;
    uint64_t mask = 0ULL;

    for (int r = rank + 1; r < 8; r++) {
        if (color == 0 && blackPieces & (bitOnBoard << ((r - rank) * 8))) {
            continue;
        } else if (color > 0 && whitePieces & (bitOnBoard << ((r - rank) * 8))) {
            continue;
        }
        mask |= 1ULL << (r * 8 + file);
    }
    for (int r = rank - 1; r > 0; r--) {
        if (color == 0 && blackPieces & (bitOnBoard >> ((rank - r) * 8))) {
            continue;
        } else if (color > 0 && whitePieces & (bitOnBoard >> ((rank - r) * 8))) {
            continue;
        }
        mask |= 1ULL << (r * 8 + file);
    }
    for (int f = file + 1; f < 7; f++) {
        if (color == 0 && blackPieces & (bitOnBoard << ((f - file) * 8))) {
            continue;
        } else if (color > 0 && whitePieces & (bitOnBoard << (f - file))) {
            continue;
        }
        mask |= 1ULL << (rank * 8 + f);
    }
    for (int f = file - 1; f > 0; f--) {
        if (color == 0 && blackPieces & (bitOnBoard >> ((f - rank) * 8))) {
            continue;
        } else if (color > 0 && whitePieces & (bitOnBoard << ((f - rank) * 8))) {
            continue;
        }
        mask |= 1ULL << (rank * 8 + f);
    }



    return mask;
}

uint64_t attack::diagonalRay(int index) {
    int rank = index / 8;
    int file = index % 8;

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

