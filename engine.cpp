#include "engine.hpp"

int Engine::mSetStartingPosition(std::string startingPosition) {

}

int Engine::mInitZobristHashing() {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 12; j++) {
        table[i][j] = random_bitstring()
        }
    }
    table.black_to_move = random_bitstring()

}

int Engine::mZobristHash(int board) {
    h := 0
    if is_black_turn(board):
        h := h XOR table.black_to_move
    for i from 1 to 64:      # loop over the board positions
        if board[i] ≠ empty:
            j := the piece at board[i], as listed in the constant indices, above
            h := h XOR table[i][j]
    return h

}

Engine::Engine() {
}

Engine::~Engine() {
}