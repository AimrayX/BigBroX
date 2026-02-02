#include "UCIHandler.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <stop_token>
#include <sstream>

#include "attack.hpp"
#include "types.hpp"

std::string UCIHandler::getStartingPosition(std::string commandLine) {
  std::string fenPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  std::stringstream ss(commandLine);
  std::string token;
  ss >> token;
  std::cout << "inside1" << std::endl;

  if(ss >> token) {
    if (token == "startpos") {
      while(ss >> token) {
        if (token == "moves") continue; 
        fenPosition += " " + token;
      }
    } else if (token == "fen") {
      while(ss >> token && token != "moves") {
        fenPosition += token + " ";
      }
    }
  }
  return fenPosition;
}

int UCIHandler::loop() {
    //start
    std::cout << "Waiting for gui to start engine..." << std::endl;
    while (currentCommand != "uci") {
        std::getline(std::cin, currentCommand);
    }
    std::cout << "id name BigBroX\nid author Hall T.\noption name Hash type spin default 16 min 1 max 1024\nuciok" << std::endl;
    
    lastCommand = currentCommand;
    while (currentCommand != "isready") {
        std::getline(std::cin, currentCommand);
    }
    
    attack::init();

    std::cout << "readyok" << std::endl;

    std::getline(std::cin, currentCommand);
  while(
    if (currentCommand[0] == 's') {
        //handle later
    } else if (currentCommand[0] == 'p') {
        std::cout << "info setting starting position" << std::endl;
        std::string startingPosition = getStartingPosition(currentCommand);
        std::cout << "here" << std::endl;
        game.position.setStartingPosition(startingPosition);
    }
    std::cout << "info starting position set" << std::endl;
    while (currentCommand[0] != 'g') {
        std::getline(std::cin, currentCommand);
    }
    std::cout << "info starting calculations" << std::endl;
    std::jthread t1([this](std::stop_token st) {this->searchResult = game.engine.search(game.position, st);});

    while (currentCommand != "stop") {
        //print engine information
        std::cout << "// INFO //" << std::endl;
        std::getline(std::cin, currentCommand);
    }
    std::cout << "info stopping engine" << std::endl;
    t1.request_stop();
    std::cout << "info engine stopped retrieving best move" << std::endl;
    Move finalMove = searchResult.load();
    std::cout << "info retrieved search result " << finalMove.to << std::endl;
    
    if(finalMove.to < Squares.size()) std::cout << "bestmove " << Squares.at(finalMove.to) << std::endl;
  
    return 0;
}

void UCIHandler::getEngineState() {
    std::cout << "info depth " << std::to_string(game.engine.mCurrentDepth.load()) << std::endl;
    std::cout << "info score cp " << std::to_string(game.engine.mCurrentEval.load()) << std::endl;
    std::cout << "info time " << std::to_string(game.engine.mTimeSpentMs.load()) << std::endl;
}

UCIHandler::UCIHandler(/* args */) {
    state = 0;
}

UCIHandler::~UCIHandler() {
}
