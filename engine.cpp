#include "engine.hpp"
#include "types.hpp"

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

int Engine::mMovePiece(std::string UCIMove) {
    uint64_t startingSquare = mAlgebraicToBit(UCIMove.substr(0,2));
    uint64_t targetSquare = mAlgebraicToBit(UCIMove.substr(2,2));

    int promotion = 0;
    if (move.length() == 5) {
        if (move[4] == 'q') {
            promotion = 1;
        } else if (move[4] == 'r') {
            promotion = 2;
        } else if (move[4] == 'b') {
            promotion = 3;
        } else if (move[4] == 'n') {
            promotion = 4;
        }  
    }
    
    mCheckIfLegal(startingSquare, targetSquare, promotion);

    mMove();
    
    
}

inline Square make_square(char file, char rank) {
    return Square((rank - '1') * 8 + (file - 'a'));
}

Move parseUCIMove(const std::string& uci) {
    Move m;
    m.from = make_square(uci[0], uci[1]);
    m.to = make_square(uci[2], uci[3]);
    m.promotion = (uci.size() == 5) ? uci[4] : 0;
    return m;
}

inline std::string squareToString(Square sq) {
    char file = 'a' + (sq % 8);
    char rank = '1' + (sq / 8);
    return std::string{file, rank};
}

uint64_t Engine::mAlgebraicToBit(std::string alge) {
    uint64_t field = 0;
    for (int i = 0; i < alge.length(); i++) {
        if (i == 0 && alge[i] == 'a') {
            field = 0b10000000LL;
        } else if (alge[i] = 'b') {
            field = 0b01000000LL;
        } else if (alge[i] = 'c') {
            field = 0b00100000LL;
        } else if (alge[i] = 'd') {
            field = 0b00010000LL;
        } else if (alge[i] = 'e') {
            field = 0b00001000LL;
        } else if (alge[i] = 'f') {
            field = 0b00000100LL;
        } else if (alge[i] = 'g') {
            field = 0b00000010LL;
        } else if (alge[i] = 'h') {
            field = 0b00000001LL;
        }
        
        if (i == 1 && alge[i] == '2') {
            field = field << 8;
        } else if (alge[i] = '3') {
            field = field << 16;
        } else if (alge[i] = '4') {
            field = field << 24;
        } else if (alge[i] = '5') {
            field = field << 32;
        } else if (alge[i] = '6') {
            field = field << 40;
        } else if (alge[i] = '7') {
            field = field << 48;
        } else if (alge[i] = '8') {
            field = field << 56;
        }
    }
    return field;
}

int Engine::mAttackGeneration(uint64_t piece) {
    uint64_t attack = 0;
    if (piece | (mWhiteRooks | mBlackRooks) == (mWhiteBishops | mBlackBishops)) {
        attack = mVertHorMask(piece);
    } else if (piece | (mWhiteBishops | mBlackBishops) == (mWhiteBishops | mBlackBishops)) {
        attack = mDiagonalMask(piece);
    } else if (piece | (mWhiteQueen | mBlackQueen) == (mWhiteQueen | mBlackQueen)) {
        attack = mDiagonalMask(piece) | mVertHorMask(piece);
    }
    
    return attack;
}

int Engine::mGetPseudoLegalMoves(uint64_t piece) {

}

int Engine::mFilterLegalMoves(uint64_t piece) {

}

int Engine::mVertHorMask(uint64_t piece) {
    int rank = __builtin_ctzll(piece) / 8;
    int file = __builtin_ctzll(piece) % 8;

    uint64_t mask = 0ULL;
    for (int r = rank + 1; r < 7; r++) mask |= 1ULL << (r * 8 + file);
    for (int r = rank - 1; r > 0; r--) mask |= 1ULL << (r * 8 + file);
    for (int f = file + 1; f < 7; f++) mask |= 1ULL << (rank * 8 + f);
    for (int f = file - 1; f > 0; f--) mask |= 1ULL << (rank * 8 + f);

    return mask;
}

int Engine::mDiagonalMask(uint64_t piece) {
    int rank = __builtin_ctzll(piece) / 8;
    int file = __builtin_ctzll(piece) % 8;

    uint64_t mask = 0ULL;
    for (int r = rank + 1, f = file + 1; r < 7 && f < 7; r++, f++) {
        mask |= 1ULL << (r * 8 + f);
    }
    for (int r = rank + 1, f = file - 1; r < 7 && f > 0; r++, f--) {
        mask |= 1ULL << (r * 8 + f);
    }
    for (int r = rank - 1, f = file + 1; r > 0 && f < 7; r--, f++) {
        mask |= 1ULL << (r * 8 + f);
    }
    for (int r = rank - 1, f = file - 1; r > 0 && f > 0; r--, f--) {
        mask |= 1ULL << (r * 8 + f);
    }

    return mask;
}

int Engine::mComputeKnightAttack() {

    return 0;
}

int Engine::mComputeKingAttack() {

    return 0;
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