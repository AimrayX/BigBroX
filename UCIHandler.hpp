#ifndef UCIHANDLER_HPP
#define UCIHANDLER_HPP

#include "game.hpp"

#include <string>

class UCIHandler {
private:
/**
 * returns the starting position as FEN + moves
 */
    std::string getStartingPosition();
public:
    Game game;
    int state;
    std::string lastCommand;
    std::string currentCommand;


    int loop();

    UCIHandler(/* args */);
    ~UCIHandler();
};



#endif