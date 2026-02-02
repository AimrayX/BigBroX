#ifndef UTILS_HPP
#define UTILS_HPP

#include "types.hpp"

#include <string>
#include <stdint.h>

namespace util {
    Move parseUCIMove(const std::string& uci);
    uint64_t mAlgebraicToBit(const std::string& alge);
    uint64_t makeSquare(char file, char rank);
    std::string squareToString(int sqr);
}



#endif
