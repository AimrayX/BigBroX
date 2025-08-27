#ifndef ATTACK_HPP
#define ATTACK_HPP

#include <stdint.h>

namespace attack {
    int mComputeKnightAttacks();
    int mComputeKingAttacks();
    extern uint64_t knightAttacks[64];
    extern uint64_t kingAttacks[64];
    uint64_t mVertHorMask(uint64_t piece);
    uint64_t mDiagonalMask(uint64_t piece);
    void init();
}

#endif