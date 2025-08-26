#include "engine.hpp"

#include <iostream>
#include <sstream>

int Engine::mSetStartingPosition(std::string startingPosition) {

    for (int i = 0; i < 71; i++) {

        if    (startingPosition[i] == '8' || startingPosition[i] == '7' 
            || startingPosition[i] == '6' || startingPosition[i] == '5' 
            || startingPosition[i] == '4' || startingPosition[i] == '3' 
            || startingPosition[i] == '2' || startingPosition[i] == '1') {
            i += startingPosition[i] - '0';
        } else if (startingPosition[i] == 'r') {
            mBlackRooks ^= 1 << (63-i);
        } else if (startingPosition[i] == 'n') {
            mBlackKnights ^= 1 << (63-i);
        } else if (startingPosition[i] == 'b') {
            mBlackBishops ^= 1 << (63-i);
        } else if (startingPosition[i] == 'q') {
            mBlackQueen ^= 1 << (63-i);
        } else if (startingPosition[i] == 'k') {
            mBlackKing ^= 1 << (63-i);
        } else if (startingPosition[i] == 'p') {
            mBlackPawns ^= 1 << (63-i);
        } else if (startingPosition[i] == 'R') {
            mWhiteRooks ^= 1 << (63-i);
        } else if (startingPosition[i] == 'N') {
            mWhiteKnights ^= 1 << (63-i);
        } else if (startingPosition[i] == 'B') {
            mWhiteBishops ^= 1 << (63-i);
        } else if (startingPosition[i] == 'Q') {
            mWhiteQueen ^= 1 << (63-i);
        } else if (startingPosition[i] == 'K') {
            mWhiteKing ^= 1 << (63-i);
        } else if (startingPosition[i] == 'P') {
            mWhitePawns ^= 1 << (63-i);
        }
        
    }
    std::istringstream iss(startingPosition);
    std::string temp;
    std::getline(iss, temp, ' ');

    std::getline(iss, temp, ' ');
    if (temp == "w") {
        mTurn = 1;
    } else {
        mTurn = 0;
    }

    std::getline(iss, temp, ' ');
    for (int i = 0; i < temp.length(); i++)
    {
        if (temp[i] == 'K') {
            mWhiteAbleCastleKing = true;
        } else if (temp[i] == 'Q') {
            mWhiteAbleCastleQueen = true;
        } else if (temp[i] == 'k') {
            mBlackAbleCastleKing = true;
        } else if (temp[i] == 'q') {
            mBlackAbleCastleQueen = true;
        }
    }

    std::getline(iss, temp, ' ');
    mEnPassentSquare = temp;
    
    std::getline(iss, temp, ' ');
    mHalfMove = std::stoi(temp);

    std::getline(iss, temp, ' ');
    mFullMove = std::stoi(temp); 

    while (iss >> temp) {
        mMovePiece(temp);
    }

    return 0;
}

int Engine::mMovePiece(std::string move) {

}

ZobristHashing::ZobristHashing(int numPieces) {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dis;

        for (int i = 0; i < numPieces; ++i) {
            pieceHashes[i] = std::vector<uint64_t>(64);
            for (int j = 0; j < 64; ++j) {
                pieceHashes[i][j] = dis(gen);
            }
        }
        currentHash = 0;
    }

void ZobristHashing::movePiece(int piece, int from, int to) {
    currentHash ^= pieceHashes[piece][from];
    currentHash ^= pieceHashes[piece][to];
}

uint64_t ZobristHashing::getHash() const {
    return currentHash;
}

Engine::Engine() {
    ZobristHashing zobrist;
}

Engine::~Engine() {
}

ZobristHashing::ZobristHashing() {

}

ZobristHashing::~ZobristHashing() {
    
}