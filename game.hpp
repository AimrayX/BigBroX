#ifndef GAME_HPP
#define GAME_HPP

#include "position.hpp"
#include "engine.hpp"
#include "types.hpp"

class Game
{
private:
    
public:
    Position position;
    Engine engine;

    int init();
    void printBoard();

    Game(/* args */);
    ~Game();
};




#endif