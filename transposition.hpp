#pragma once

#include "types.hpp"
#include <vector>

struct TTEntry {
    uint64_t key;   // Zobrist hash (to verify this entry belongs to this position)
    Move move;      // Best move found
    int score;      // Evaluation score
    int depth;      // How deep we searched
    uint8_t flag;   // EXACT, ALPHA, or BETA
    uint8_t generation; // (Optional) For aging, helpful later
    
    TTEntry() : key(0), move(), score(0), depth(0), flag(0), generation(0) {}
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
    std::vector<TTEntry> table;
    size_t size; // Number of entries
    
    // Helper to adjust mate scores so they are valid across different plys
    int scoreToTT(int score, int ply);
    int scoreFromTT(int score, int ply);
};
