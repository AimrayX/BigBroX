#ifndef ATTACK_HPP
#define ATTACK_HPP

#include <stdint.h>

namespace attack {
    void computeKnightAttacks();
    void computeKingAttacks();
    extern uint64_t knightAttacks[64];
    extern uint64_t kingAttacks[64];
    uint64_t vertHorRay(uint64_t piece, const uint64_t& whitePieces,const uint64_t& blackPieces);
    uint64_t diagonalRay(uint64_t piece, uint64_t whitePieces, uint64_t blackPieces);
    void init();
}

#endif