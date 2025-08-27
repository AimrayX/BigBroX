#ifndef GAME_HPP
#define GAME_HPP

#include "position.hpp"
#include "engine.hpp"
#include "types.hpp"

class Game
{
private:
    /* data */
public:
    Position position;
    Engine engine;

    void start();
    void printBoard();

    Game(/* args */);
    ~Game();
};




#endif