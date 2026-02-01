#ifndef UTILS_HPP
#define UTILS_HPP

#include "types.hpp"

#include <string>
#include <stdint.h>

namespace util {
    Move parseUCIMove(const std::string& uci);
    uint64_t mAlgebraicToBit(std::string alge);
}



#endif
