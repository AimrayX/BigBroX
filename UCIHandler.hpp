#ifndef UCIHANDLER_HPP
#define UCIHANDLER_HPP

#include "game.hpp"

#include <string>
#include <thread>

class UCIHandler {
private:
/**
 * returns the starting position as FEN + moves
 */
    std::string getStartingPosition();
    std::atomic<Move> searchResult;

public:
    Game game;
    int state;
    std::string lastCommand;
    std::string currentCommand;

    int loop();
    void getEngineState();
    std::string moveToString(Move move);

    UCIHandler(/* args */);
    ~UCIHandler();
};



#endif
