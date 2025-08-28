#ifndef UTILS_HPP
#define UTILS_HPP

#include "types.hpp"

#include <string>
#include <stdint.h>

namespace util {
    Square make_square(char file, char rank);
    std::string squareToString(Square sq);
    Move parseUCIMove(const std::string& uci);
    uint64_t mAlgebraicToBit(std::string alge);
}



#endif