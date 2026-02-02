#include "game.hpp"

void Game::printBoard() {
    for (int r = 7; r >= 0; r--) { // rank 8 -> 1
        for (int f = 0; f < 8; f++) { // files A->H
            int square = r * 8 + f;
            std::cout << ((position.occupancies[2] >> square) & 1);
        }
        std::cout << "\n";
    }
}

Game::Game(){
}

Game::~Game() {
}
