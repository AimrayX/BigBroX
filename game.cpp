#include "game.hpp"


int Game::init() {

}

void Game::printBoard() {
    for (int r = 7; r >= 0; r--) { // rank 8 -> 1
        for (int f = 0; f < 8; f++) { // files A->H
            int square = r * 8 + f;
            std::cout << ((position.getAllOccupiedSquares() >> square) & 1);
        }
        std::cout << "\n";
    }
}

Game::Game(/* args */) {
}

Game::~Game() {
}