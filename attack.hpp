#pragma once

#include <stdint.h>

namespace attack {
    void computeKnightAttacks();
    void computeKingAttacks();
    void initRays();
    void init();
    extern uint64_t knightAttacks[64];
    extern uint64_t kingAttacks[64];
    extern uint64_t Rays[8][64];
    uint64_t getRookAttacks(int square, uint64_t occupancy);
    uint64_t getBishopAttacks(int square, uint64_t occupancy);
    uint64_t getKingAttacks(int square, uint64_t occupancy);
    uint64_t getQueenAttacks(int square, uint64_t occupancy);
    uint64_t getKnightAttacks(int square, uint64_t occupancy);
    uint64_t getPawnAttacks(int square, uint64_t occupancy);
}

