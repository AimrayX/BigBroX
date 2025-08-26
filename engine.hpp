#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <string>

class Engine
{
private:
    /* data */
public:
    int mWhitePawns;
    int mDepth;
    std::string mStartingPosition;
    std::string mLastBestMove;
    int table[64][12];

    int mInitZobristHashing();
    int mZobristHash(int board);
    int mSetStartingPosition(std::string startingPosition);
    std::string mSearch();

    Engine();
    ~Engine();
};




#endif