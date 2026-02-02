#include "utils.hpp"
#include "types.hpp"

uint64_t util::makeSquare(char file, char rank) {
    return ((rank - '1') * 8 + (file - 'a'));
}

Move util::parseUCIMove(const std::string& uci) {
    Move m;
    m.from = makeSquare(uci[0], uci[1]);
    m.to = makeSquare(uci[2], uci[3]);

    if (uci.size() == 5) {
        char p = uci[4];
        if (p == 'q') m.promotion = QUEEN;
        else if (p == 'r') m.promotion = ROOK;
        else if (p == 'b') m.promotion = BISHOP;
        else if (p == 'n') m.promotion = KNIGHT;
    } else {
        m.promotion = NOPIECE; 
    }
    return m;
}

std::string util::squareToString(int sq) {
    char file = 'a' + (sq % 8);
    char rank = '1' + (sq / 8);
    return std::string{file, rank};
}

uint64_t util::mAlgebraicToBit(const std::string& alge) {
    if (alge == "-") return 0;

    int square = makeSquare(alge[0], alge[1]);
    return (1ULL << square);
}
