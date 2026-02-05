#pragma once

#include <vector>

#include "types.hpp"

struct alignas(16) TTEntry {
  uint64_t key;
  TTMove move;
  int16_t score;
  uint8_t depth;
  uint8_t flag;
  uint8_t generation;
  uint8_t pad;

  TTEntry() : key(0), move(), score(0), depth(0), flag(0), generation(0), pad(0) {}
};

struct alignas(64) TTBucket {
  TTEntry entries[4];
};

class TranspositionTable {
 public:
  TranspositionTable(int sizeInMB);
  ~TranspositionTable();

  void clear();
  // Store a result in the table
  void store(uint64_t key, int depth, int ply, int score, uint8_t flag, Move move);
  // Retrieve a result (returns true if found and usable)
  bool probe(uint64_t key, int depth, int ply, int alpha, int beta, int& outScore, Move& outMove);
  // Just retrieve the move (for move ordering)
  Move probeMove(uint64_t key);

 private:
  std::vector<TTBucket> buckets;
  size_t numBuckets;  // Number of entries
  int scoreToTT(int score, int ply);
  int scoreFromTT(int score, int ply);
};
