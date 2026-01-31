#include "UCIHandler.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <stop_token>

#include "attack.hpp"

std::string UCIHandler::getStartingPosition() {
    lastCommand = currentCommand;
    std::string tempCommand;
    while (tempCommand != "position") {
        std::getline(std::cin, tempCommand, ' ');
    }
    currentCommand = tempCommand;
    std::cin >> tempCommand;

    if (tempCommand == "startpos") {
        char nextChar = std::cin.peek(); 

        if (nextChar != '\n' && nextChar != EOF) {
            std::cin >> tempCommand;
            if (tempCommand == "moves") {
                std::string movesLine;
                std::getline(std::cin, movesLine);
                currentCommand = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 " + movesLine;
            } 
        } else {
            currentCommand = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 \n";
        }

    } else if (currentCommand == "fen") {
        std::string fen;
        std::getline(std::cin, tempCommand, ' ');
        fen = tempCommand;
        std::getline(std::cin, tempCommand, ' ');
        fen = fen + " " + tempCommand;
        std::getline(std::cin, tempCommand, ' ');
        fen = fen + " " + tempCommand;
        std::getline(std::cin, tempCommand, ' ');
        fen = fen + " " + tempCommand;
        std::getline(std::cin, tempCommand, ' ');
        fen = fen + " " + tempCommand;
        std::cin >> tempCommand;
        fen = fen + " " + tempCommand;

        char nextChar = std::cin.peek(); 
        if (nextChar != '\n' && nextChar != EOF) {
            
            std::cin >> tempCommand;
            if (tempCommand == "moves") {
                std::string movesLine;
                std::getline(std::cin, movesLine);
                currentCommand = fen + " " + movesLine;
            } 
        } else {
          currentCommand = fen + "\n";
        }
    }
  return currentCommand;
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
    if (currentCommand[0] == 's') {
        //handle later
    } else if (currentCommand[0] == 'p') {
        std::string startingPosition = getStartingPosition();
        game.position.setStartingPosition(startingPosition);
    }
    
    while (currentCommand[0] != 'g') {
        std::getline(std::cin, currentCommand);
    }

    std::jthread t1([this](std::stop_token st) {this->searchResult = game.engine.search(game.position, st);});

    while (currentCommand != "stop") {
        //print engine information
        getEngineState();
        std::getline(std::cin, currentCommand);
    }
    t1.request_stop();

    std::cout << "bestmove" << moveToString(searchResult) << std::endl;
  
    return 0;
}

void UCIHandler::getEngineState() {
    std::cout << "info depth " << std::to_string(game.engine.mCurrentDepth.load()) << std::endl;
    std::cout << "info score cp " << std::to_string(game.engine.mCurrentEval.load()) << std::endl;
    std::cout << "info time " << std::to_string(game.engine.mTimeSpentMs.load()) << std::endl;
}

std::string UCIHandler::moveToString(Move move) {
  std::string strMove = "";

  return strMove;
}

UCIHandler::UCIHandler(/* args */) {
    state = 0;
}

UCIHandler::~UCIHandler() {
}
