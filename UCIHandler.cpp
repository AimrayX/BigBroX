#include "UCIHandler.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <stop_token>
#include <sstream>
#include <unordered_map>

#include "attack.hpp"
#include "types.hpp"

std::string UCIHandler::getStartingPosition(std::string commandLine) {
  std::string fenPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  std::stringstream ss(commandLine);
  std::string token;
  ss >> token;

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
    static const std::unordered_map<std::string, UCICommand> commandMap = {
      {"uci",        UCICommand::Uci},
      {"isready",    UCICommand::IsReady},
      {"setoption",  UCICommand::SetOption},
      {"ucinewgame", UCICommand::UCINewGame},
      {"position",   UCICommand::Position},
      {"go",         UCICommand::Go},
      {"stop",       UCICommand::Stop},
      {"quit",       UCICommand::Quit}
    };

  std::string line;
  std::string token;
  while(std::getline(std::cin, line)) {

    std::stringstream ss(line);

    if(ss >> token) {

      auto it = commandMap.find(token);

      if (it != commandMap.end()) {
  
        switch(it->second) {
          case UCICommand::Uci:
            std::cout << "id name BigBroX\nid author Hall T.\noption name Hash type spin default 16 min 1 max 1024\nuciok" << std::endl;
            break;

          case UCICommand::IsReady:
            attack::init();
            std::cout << "readyok" << std::endl;
            break;

          case UCICommand::Position: {
            std::cout << "info setting starting position" << std::endl;
            std::string startingPosition = getStartingPosition(line);
            game.position.setStartingPosition(startingPosition);
            break;
          }

          case UCICommand::Go: {
            std::cout << "info starting calculations" << std::endl;

            if (t1.joinable()) t1.request_stop();

            t1 = std::jthread([this](std::stop_token st) {this->searchResult = game.engine.search(game.position, st);});
            break;
          }

          case UCICommand::Stop: {
            std::cout << "info stopping engine" << std::endl;
            t1.request_stop();

            if(t1.joinable()) t1.join();

            std::cout << "info engine stopped retrieving best move" << std::endl;
            Move finalMove = searchResult.load();

            std::cout << "info retrieved search result " << (int)finalMove.from << (int)finalMove.to << std::endl;
            if(finalMove.to < Squares.size()) std::cout << "bestmove " << Squares.at((int)finalMove.from) <<Squares.at((int)finalMove.to) << std::endl;
            break;
          }

          case UCICommand::Quit:
            std::cout << "quitting" << std::endl;
            return 0;
        }
      } else { std::cout << "unknown command" << std::endl; return -1; }
    }
  }
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
