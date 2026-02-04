#pragma once

#include <cstdint>
#include <string_view>
#include <array>

enum Piece {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NOPIECE
};

enum CastlingRights {
    WHITE_OO = 1,
    WHITE_OOO = 2,
    BLACK_OO = 4,
    BLACK_OOO = 8
};

enum Color { 
    WHITE, 
    BLACK, 
    COLOR_NB 
};

enum TTFlag {
  TT_EXACT,
  TT_ALPHA,
  TT_BETA
};

struct Move {
    uint8_t from;
    uint8_t to;
    uint8_t promotion;
    uint8_t padding;
    int score;

    Move() = default; 

    Move(uint8_t dFrom, uint8_t dTo, uint8_t dPromotion) 
      : from(dFrom), to(dTo), promotion(dPromotion), padding(0), score(0) {}

    static Move null() {
        Move m;
        m.from = 0;
        m.to = 0;
        m.promotion = NOPIECE;
        return m;
    }
};

struct MoveList {
  Move moves[256];
  int count = 0;

  inline void add(const Move& m, int score) {
    moves[count] = m;
    moves[count].score = score;
    count++;
  }

  inline void clear() {
    count = 0;
  }
};

struct StateInfo {
  int capturedPiece;
  int movedPiece;
  int promotedToPiece;
  uint64_t promotionSquare;
  int castle;
  uint64_t epSquare;
  int halfMove;

  int psqtScore;

  uint64_t zobristKey;
};

inline constexpr std::array<std::string_view, 64> Squares = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};

static constexpr int pieceValues[6] = {
        100,   // PAWN
        300,   // KNIGHT
        320,   // BISHOP
        500,   // ROOK
        900,   // QUEEN
        20000  // KING
    };

    static constexpr int PST[6][64] = {
    {
      0,0,0,0,0,0,0,0,
      0,0,-5,-10,-10,-5,0,0,
      5,0,5,10,10,5,0,5,
      0,0,10,20,20,10,0,0,
      0,0,10,20,20,10,0,0,
      0,0,5,10,10,5,0,0,
      5,5,5,10,10,5,5,5,
      20,20,20,20,20,20,20,20,
    },
    {
      -50,-10,-10,-10,-10,-10,-10,-50,
      -10,-5,0,5,5,0,-5,-10,
      -5,0,10,10,10,10,0,-5,
      -5,0,10,20,20,10,0,-5,
      -5,0,10,25,25,10,0,-5,
      -10,0,5,10,10,5,0,-10,
      -10,-5,0,0,0,0,-5,-10,
      -50,-10,-10,-10,-10,-10,-10,-50,
    },
    {
      -10,-10,-10,-10,-10,-10,-10,-10,
       10,10,0,0,0,0,10,10,
       5,20,0,10,10,0,20,5,
       0,0,20,20,20,20,0,0,
       0,0,10,10,10,10,0,0,
       -5,0,5,10,10,5,0,-5,
      -10,-5,-5,-5,-5,-5,-5,-10,
      -10,-10,-10,-10,-10,-10,-10,-10,
    },
    {
       -10,10,10,20,20,10,10,-10,
       0,5,5,10,10,5,5,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       10,10,10,10,10,10,10,10,
       0,0,0,0,0,0,0,0,
    },
    {
      -10,0,0,-5,0,0,0,-10,
       0,0,0,10,5,0,0,0,
       5,10,0,5,0,0,10,5,
       0,0,5,5,5,5,0,0,
       0,0,0,5,5,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
    },
    {
       20,10,0,-5,-5,0,10,20,
       10,0,-5,-5,-5,-5,0,10,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
    }
    };
