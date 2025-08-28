#include "utils.hpp"

Square util::make_square(char file, char rank) {
    return Square((rank - '1') * 8 + (file - 'a'));
}

Move util::parseUCIMove(const std::string& uci) {
    Move m;
    m.from = make_square(uci[0], uci[1]);
    m.to = make_square(uci[2], uci[3]);
    m.promotion = (uci.size() == 5) ? uci[4] : 0;
    return m;
}

std::string util::squareToString(Square sq) {
    char file = 'a' + (sq % 8);
    char rank = '1' + (sq / 8);
    return std::string{file, rank};
}

uint64_t util::mAlgebraicToBit(std::string alge) {
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