#pragma once

#include <stdint.h>

namespace attack {
    void computeKnightAttacks();
    void computeKingAttacks();
    extern uint64_t knightAttacks[64];
    extern uint64_t kingAttacks[64];
    uint64_t vertHorMask(uint64_t piecePos);
    uint64_t diagonalMask(uint64_t piecePos);
    void init();
}

