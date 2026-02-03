#include "transposition.hpp"
#include <iostream>

// Threshold to detect if a score is a mate score (must match your INF)
static const int MATE_BOUND = 900000; 
static const int INF_SCORE = 1000000;

TranspositionTable::TranspositionTable(int sizeInMB) {
    // Calculate how many entries fit in the given MB
    // Size = (MB * 1024 * 1024) / sizeof(TTEntry)
    size_t bytes = sizeInMB * 1024 * 1024;
    size = bytes / sizeof(TTEntry);
    table.resize(size);
    std::cout << "TT Initialized with " << size << " entries." << std::endl;
}

TranspositionTable::~TranspositionTable() {}

void TranspositionTable::clear() {
    std::fill(table.begin(), table.end(), TTEntry());
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

void TranspositionTable::store(uint64_t key, int depth, int ply, int score, uint8_t flag, Move move) {
    // Map hash to index
    size_t index = key % size;
    
    // Replacement Strategy:
    // Always replace if:
    // 1. The existing entry is from a different position (key collision or empty)
    // 2. OR the new search is deeper (more valuable)
    // 3. OR the new score is EXACT (very valuable)
    
    // Simple "Deepest or New" strategy:
    if (table[index].key != key || depth >= table[index].depth) {
        table[index].key = key;
        table[index].depth = depth;
        table[index].flag = flag;
        table[index].score = scoreToTT(score, ply);
        table[index].move = move;
    }
}

bool TranspositionTable::probe(uint64_t key, int depth, int ply, int alpha, int beta, int& outScore, Move& outMove) {
    size_t index = key % size;
    const TTEntry& entry = table[index];

    // Check if the key matches (verify it's the same position)
    if (entry.key == key) {
        
        // Always return the move for ordering, even if depth is insufficient
        outMove = entry.move; 

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
    }
    return false;
}

Move TranspositionTable::probeMove(uint64_t key) {
    size_t index = key % size;
    if (table[index].key == key) {
        return table[index].move;
    }
    return Move(); // Return empty move if not found
}
