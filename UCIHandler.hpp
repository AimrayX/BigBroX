#ifndef UCIHANDLER_HPP
#define UCIHANDLER_HPP

#include "game.hpp"

#include <string>
#include <thread>

enum class UCICommand {
  Uci,
  IsReady,
  SetOption,
  UCINewGame,
  Position,
  Go,
  Stop,
  Quit,
  Unknown
};

class UCIHandler {
private:
/**
 * returns the starting position as FEN + moves
 */
    std::string getStartingPosition(std::string commandLine);
    std::atomic<Move> searchResult;
    std::jthread t1;

public:
    Game game;
    int state;
    std::string lastCommand;
    std::string currentCommand;

    int loop();
    void getEngineState();

    UCIHandler(/* args */);
    ~UCIHandler();
};



#endif
