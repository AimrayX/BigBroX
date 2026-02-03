#include "UCIHandler.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <thread>
#include <stop_token>
#include <sstream>
#include <unordered_map>

#include "attack.hpp"
#include "types.hpp"
#include "utils.hpp"

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
      fenPosition = "";
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
            std::cout << "id name BigBroX 1.0" << std::endl;
            std::cout << "id author Hall T." << std::endl;
            //std::cout << "option name Hash type spin default 16 min 1 max 1024" << std::endl;
            std::cout << "uciok" << std::endl;
            break;

          case UCICommand::IsReady:
            attack::init();
            std::cout << "readyok" << std::endl;
            break;

          case UCICommand::UCINewGame:
            game.engine.tt.clear();
            break;

          case UCICommand::SetOption:
            break;

          case UCICommand::Position: {
            // 1. Setup Variables
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::vector<std::string> moveStrings; // Store moves here temporarily
    std::string subToken;
    ss >> subToken; // Read "startpos" or "fen"

    // 2. Determine Base Position
    if (subToken == "startpos") {
        std::string buffer;
        while (ss >> buffer) {
            if (buffer == "moves") {
                // Found "moves", read the rest into our vector
                while (ss >> buffer) moveStrings.push_back(buffer);
                break;
            }
        }
    } 
    else if (subToken == "fen") {
        fen = "";
        std::string buffer;
        bool readingFen = true;
        while (ss >> buffer) {
            if (buffer == "moves") {
                readingFen = false;
                while (ss >> buffer) moveStrings.push_back(buffer);
                break; 
            }
            if (readingFen) {
                fen += (fen.empty() ? "" : " ") + buffer;
            }
        }
    }

    // 3. Set the Board State (Base)
        game.position.setStartingPosition(fen);

    // 4. Apply Moves (The Matcher Logic)
    // This runs for BOTH startpos and fen cases
    for (const std::string& moveStr : moveStrings) {
        // A. Parse the simple target coordinates
        Move target = util::parseUCIMove(moveStr); 
        // B. Generate legal moves to find the one with correct flags
        MoveList legalMoves;
        game.position.getMoves(game.position.mSideToMove, legalMoves);
        bool found = false;
        for (int i = 0; i < legalMoves.count; i++) {
            Move m = legalMoves.moves[i];
            // C. Match coordinates + promotion
            if (m.from == target.from && 
                m.to == target.to && 
                m.promotion == target.promotion) {
                game.position.doMove(m); // Apply the fully populated move
                found = true;
                break;
            }
        }
        if (!found) {
            std::cout << "Debug: Could not match move " << moveStr << std::endl;
        }
    }

            break;
          }
          case UCICommand::Go: {

            if (t1.joinable()) t1.request_stop();
            int wtime = 0;
            int btime = 0;
            int winc = 0;
            int binc = 0;
            int movetime = 0;

            while(ss >> token) {
              if(token == "wtime") {
                ss >> token;
                wtime = std::stoi(token);
              } else if(token == "btime") {
                ss >> token;
                btime = std::stoi(token);
              } else if(token == "winc") {
                ss >> token;
                winc = std::stoi(token);
              } else if(token == "binc") {
                ss >> token;
                binc = std::stoi(token);
              } else if(token == "winc") {
                ss >> token;
                winc = std::stoi(token);
              } else if(token == "movetime") {
                ss >> token;
                movetime = std::stoi(token);
              } else if(token == "infinite") {
                movetime = 100000;
              }
            }
            int allocatedTime = movetime;
            if(!movetime) allocatedTime = (game.position.mSideToMove == WHITE) ? ((wtime / 30) + winc) : ((btime / 30) + binc);
            allocatedTime = std::max(10, allocatedTime -50);
            //std::cout << "allocated Time: " << allocatedTime << std::endl;
            t1 = std::jthread([this, allocatedTime](std::stop_token st) {game.engine.search(game.position, allocatedTime, st);});
            break;
          }

          case UCICommand::Stop: {
            if(t1.joinable()) { t1.request_stop(); t1.join(); }
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


UCIHandler::UCIHandler(/* args */) {
    state = 0;
}

UCIHandler::~UCIHandler() {
}
