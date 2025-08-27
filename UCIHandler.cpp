#include "UCIHandler.hpp"

#include <iostream>
#include <string>

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
                return currentCommand;
            } 
        }
        currentCommand = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 \n";
        return currentCommand;

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
                return currentCommand;
            } 
        }
        currentCommand = fen + "\n";
        return currentCommand;
    }
}

int UCIHandler::loop() {
    //start
    std::cout << "Waiting for gui to start engine..." << std::endl;
    while (currentCommand != "uci")
    {
        std::getline(std::cin, currentCommand);
    }
    std::cout << "id name BigBroX\nid author Hall T.\noption name Hash type spin default 16 min 1 max 1024\nuciok" << std::endl;
    
    lastCommand = currentCommand;
    while (currentCommand != "isready")
    {
        std::getline(std::cin, currentCommand);
    }

    std::string startingPosition = getStartingPosition();
    game.position.setStartingPosition(startingPosition);

}


UCIHandler::UCIHandler(/* args */) {
    state = 0;
}

UCIHandler::~UCIHandler() {
}