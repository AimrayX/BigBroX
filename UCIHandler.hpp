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
    void getEngineState(std::stop_token stoken);

    UCIHandler(/* args */);
    ~UCIHandler();
};



#endif