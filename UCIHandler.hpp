#ifndef UCIHANDLER_HPP
#define UCIHANDLER_HPP

#include "engine.hpp"

#include <string>

class UCIHandler {
private:
/**
 * returns the starting position as FEN + moves
 */
    std::string getStartingPosition();
public:
    int state;
    std::string lastCommand;
    std::string currentCommand;
    Engine engine;

    int start();

    UCIHandler(/* args */);
    ~UCIHandler();
};



#endif