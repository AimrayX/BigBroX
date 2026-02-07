#include "transposition.hpp"

#include <iostream>

static const int MATE_BOUND = 900000;

TranspositionTable::TranspositionTable(int sizeInMB) {
  size_t bytes = sizeInMB * 1024 * 1024;

  size_t targetBuckets = bytes / sizeof(TTBucket);

  numBuckets = 1;
  while (numBuckets * 2 <= targetBuckets) {
    numBuckets *= 2;
  }

  // Resize vector. std::vector usually respects alignment of the type.
  buckets.resize(numBuckets);

  std::cout << "TT Initialized with " << numBuckets << " buckets (" << numBuckets * 4
            << " entries)." << std::endl;
}

TranspositionTable::~TranspositionTable() {}

void TranspositionTable::clear() {
  // Re-filling with empty buckets
  std::fill(buckets.begin(), buckets.end(), TTBucket());
}

int TranspositionTable::scoreToTT(int score, int ply) {
  if (score > MATE_BOUND) return score + ply;
  if (score < -MATE_BOUND) return score - ply;
  return score;
}

int TranspositionTable::scoreFromTT(int score, int ply) {
  if (score > MATE_BOUND) return score - ply;
  if (score < -MATE_BOUND) return score + ply;
  return score;
}

void TranspositionTable::store(uint64_t key, int depth, int ply, int score, uint8_t flag,
                               Move move) {
  // Use the key to find the bucket index
  TTBucket& bucket = buckets[key & (numBuckets - 1)];

  int replaceIndex = -1;
  int lowestDepth = 1000;

  for (int i = 0; i < 4; i++) {
    const TTEntry& entry = bucket.entries[i];
    if (entry.key == key || entry.key == 0) {
      replaceIndex = i;
      break;
    }
    if (entry.depth < lowestDepth) {
      lowestDepth = entry.depth;
      replaceIndex = i;
    }
  }
  if (replaceIndex == -1) replaceIndex = 0;  // Safety fallback

  TTEntry& entry = bucket.entries[replaceIndex];
  entry.key = key;
  entry.move = TTMove(move);
  entry.score = scoreToTT(score, ply);
  entry.depth = (uint8_t)depth;
  entry.flag = flag;
  entry.generation = 1;
}

bool TranspositionTable::probe(uint64_t key, int depth, int ply, int alpha, int beta, int& outScore,
                               Move& outMove) {
  const TTBucket& bucket = buckets[key & (numBuckets - 1)];

  for (int i = 0; i < 4; i++) {
    const TTEntry& entry = bucket.entries[i];

    if (entry.key == key) {
      outMove = entry.move.toMove();

      if (entry.depth >= depth) {
        int score = scoreFromTT(entry.score, ply);
        if (entry.flag == TT_EXACT) {
          outScore = score;
          return true;
        }
        if (entry.flag == TT_ALPHA && score <= alpha) {
          outScore = alpha;
          return true;
        }
        if (entry.flag == TT_BETA && score >= beta) {
          outScore = beta;
          return true;
        }
      }
      return false;  // Found key, but depth too low
    }
  }
  return false;
}

Move TranspositionTable::probeMove(uint64_t key) {
  const TTBucket& bucket = buckets[key & (numBuckets - 1)];

  for (int i = 0; i < 4; i++) {
    if (bucket.entries[i].key == key) {
      return bucket.entries[i].move.toMove();
    }
  }
  return Move::null();
}
