#include "attack.hpp"
#include <cstdint>

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

void attack::initRays() {
  for(int sq = 0; sq < 64; sq++) {
    int r = sq / 8;
    int f = sq % 8;

    for(int i = 1; i < 8; i++) {
      if(r + i > 7) break;

      int target = (r + i) * 8 + f;
      Rays[0][sq] |= (1ULL << target);
    }

    for(int i = 1; i < 8; i++) {
      if(r - i < 0) break;

      int target = (r - i) * 8 + f;
      Rays[1][sq] |= (1ULL << target);
    }

    for(int i = 1; i < 8; i++) {
      if(f + i > 7) break;

      int target = r * 8 + (f + i);
      Rays[2][sq] |= (1ULL << target);
    }

    for(int i = 1; i < 8; i++) {
      if(r - i < 0) break;

      int target = r * 8 + (f - i);
      Rays[3][sq] |= (1ULL << target);
    }

    for(int i = 1; i < 8; i++) {
      if(r + i > 7 || f + i > 7) break;

      int target = (r + i) * 8 + (f + i);
      Rays[4][sq] |= (1ULL << target);
    }

    for(int i = 1; i < 8; i++) {
      if(r + i > 7 || f - i < 0) break;

      int target = (r + i) * 8 + (f - i);
      Rays[5][sq] |= (1ULL << target);
    }

    for(int i = 1; i < 8; i++) {
      if(r - i < 0 || f + i > 7) break;

      int target = (r - i) * 8 + (f + i);
      Rays[6][sq] |= (1ULL << target);
    }

    for(int i = 1; i < 8; i++) {
      if(r - i < 0 || f - i < 0) break;

      int target = (r - i) * 8 + (f - i);
      Rays[7][sq] |= (1ULL << target);
    }
  }
}

uint64_t attack::getBishopAttacks(int square, uint64_t occupancy) {
  uint64_t attacks = 0ULL;

  uint64_t ne_ray = Rays[4][square];
  uint64_t ne_blockers = ne_ray & occupancy;

  if(ne_blockers == 0) {
    attacks |= ne_ray;
  } else {
    int blocker = __builtin_ctzll(ne_blockers);
    attacks |= ne_ray ^ Rays[4][blocker];
    attacks |= (1ULL << blocker);
  }

  uint64_t nw_ray = Rays[5][square];
  uint64_t nw_blockers = nw_ray & occupancy;

  if(nw_blockers == 0) {
    attacks |= nw_ray;
  } else {
    int blocker = __builtin_ctzll(nw_blockers);
    attacks |= nw_ray ^ Rays[5][blocker];
    attacks |= (1ULL << blocker);
  }

  uint64_t se_ray = Rays[6][square];
  uint64_t se_blockers = se_ray & occupancy;

  if(se_blockers == 0) {
    attacks |= se_ray;
  } else {
    int blocker = 63 - __builtin_clzll(se_blockers);
    attacks |= se_ray ^ Rays[6][blocker];
    attacks |= (1ULL << blocker);
  }

  uint64_t sw_ray = Rays[7][square];
  uint64_t sw_blockers = sw_ray & occupancy;

  if(sw_blockers == 0) {
    attacks |= sw_ray;
  } else {
    int blocker = 63 - __builtin_clzll(sw_blockers);
    attacks |= sw_ray ^ Rays[7][blocker];
    attacks |= (1ULL << blocker);
  }

  return attacks;
}

uint64_t attack::getRookAttacks(int square, uint64_t occupancy) {
  uint64_t attacks = 0ULL;

  uint64_t n_ray = Rays[0][square];
  uint64_t n_blockers = n_ray & occupancy;

  if(n_blockers == 0) {
    attacks |= n_ray;
  } else {
    int blocker = __builtin_ctzll(n_blockers);
    attacks |= n_ray ^ Rays[0][blocker];
    attacks |= (1ULL << blocker);
  }

  uint64_t e_ray = Rays[2][square];
  uint64_t e_blockers = e_ray & occupancy;

  if(e_blockers == 0) {
    attacks |= e_ray;
  } else {
    int blocker = __builtin_ctzll(e_blockers);
    attacks |= e_ray ^ Rays[2][blocker];
    attacks |= (1ULL << blocker);
  }

  uint64_t s_ray = Rays[1][square];
  uint64_t s_blockers = s_ray & occupancy;

  if(s_blockers == 0) {
    attacks |= s_ray;
  } else {
    int blocker = 63 - __builtin_clzll(s_blockers);
    attacks |= s_ray ^ Rays[1][blocker];
    attacks |= (1ULL << blocker);
  }

  uint64_t w_ray = Rays[3][square];
  uint64_t w_blockers = w_ray & occupancy;

  if(w_blockers == 0) {
    attacks |= w_ray;
  } else {
    int blocker = 63 - __builtin_clzll(w_blockers);
    attacks |= w_ray ^ Rays[3][blocker];
    attacks |= (1ULL << blocker);
  }

  return attacks;
}

uint64_t attack::getQueenAttacks(int square, uint64_t occupancy) {

  return (attack::getRookAttacks(square, occupancy) | attack::getBishopAttacks(square, occupancy));
}


namespace attack {
    uint64_t knightAttacks[64];
    uint64_t kingAttacks[64];
    uint64_t Rays[8][64];
}

void attack::init() {
    computeKnightAttacks();
    computeKingAttacks();
    initRays();
}

