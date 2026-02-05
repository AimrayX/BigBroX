#pragma once

#include <array>
#include <cstdint>
#include <string_view>

enum Piece { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NOPIECE };

enum CastlingRights { WHITE_OO = 1, WHITE_OOO = 2, BLACK_OO = 4, BLACK_OOO = 8 };

enum Color { WHITE, BLACK, COLOR_NB };

enum TTFlag { TT_EXACT, TT_ALPHA, TT_BETA };

struct Move {
  uint8_t from;
  uint8_t to;
  uint8_t promotion;
  uint8_t padding;
  int32_t score;

  Move() = default;

  Move(uint8_t dFrom, uint8_t dTo, uint8_t dPromotion)
      : from(dFrom), to(dTo), promotion(dPromotion), padding(0), score(0) {}

  static Move null() { return Move(); }
};

struct TTMove {
  uint16_t data;

  // Default to a value that decodes to Move::null() (from=0, to=0, promo=NOPIECE)
  // NOPIECE is 6. 6 << 12 = 0x6000.
  TTMove() : data(0x6000) {}

  TTMove(const Move& m) {
    data = (m.from & 0x3F) | ((m.to & 0x3F) << 6) | ((m.promotion & 0xF) << 12);
  }

  Move toMove() const {
    // If data matches our null signature, return null
    if (data == 0x6000) return Move::null();

    int from = data & 0x3F;
    int to = (data >> 6) & 0x3F;
    int promo = (data >> 12) & 0xF;
    return Move(from, to, promo);
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

  inline void clear() { count = 0; }
};

// OPTIMIZED: Eval caching structure
struct EvalCache {
  bool valid = false;
  int16_t pawnScore[2];      // [WHITE], [BLACK]
  int16_t safetyScore[2];    // [WHITE], [BLACK]
  int16_t mobilityScore[2];  // [WHITE], [BLACK]

  void invalidate() { valid = false; }
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
  EvalCache evalCache;
};

struct Eval {
  int positionScore;
  int pawnStructureScore;
  int pieceMobilityScore;
};

inline constexpr std::array<std::string_view, 64> Squares = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3", "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5", "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7", "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"};

static constexpr int pieceValues[6] = {
    100,   // PAWN
    300,   // KNIGHT
    320,   // BISHOP
    500,   // ROOK
    900,   // QUEEN
    20000  // KING
};

static constexpr int PST[6][64] = {
    // PAWN
    {
        0,   0,  0,   0,   0,   0,  0,  0,   98,  134, 61, 95,  68, 126, 34, -11,
        -6,  7,  26,  31,  65,  56, 25, -20, -14, 13,  6,  21,  23, 12,  17, -23,
        -27, -2, -5,  12,  17,  6,  10, -25, -26, -4,  -4, -10, 3,  3,   33, -12,
        -35, -1, -20, -23, -15, 24, 38, -22, 0,   0,   0,  0,   0,  0,   0,  0,
    },
    // KNIGHT
    {
        -167, -89, -34, -49, 61, -97, -15, -107, -73,  -41, 72,  36,  23,  62,  7,   -17,
        -47,  60,  37,  65,  84, 129, 73,  44,   -9,   17,  19,  53,  37,  69,  18,  22,
        -13,  4,   16,  13,  28, 19,  21,  -8,   -23,  -9,  12,  10,  19,  17,  25,  -16,
        -29,  -53, -12, -3,  -1, 18,  -14, -19,  -105, -21, -58, -33, -17, -28, -19, -23,
    },
    // BISHOP
    {
        -29, 4,  -82, -37, -25, -42, 7,  -8, -26, 16, -18, -13, 30,  59,  18,  -47,
        -16, 37, 43,  40,  35,  50,  37, -2, -4,  5,  19,  50,  37,  37,  7,   -2,
        -6,  13, 13,  26,  34,  12,  10, 4,  0,   15, 15,  15,  14,  27,  18,  10,
        4,   15, 16,  0,   7,   21,  33, 1,  -33, -3, -14, -21, -13, -12, -39, -21,
    },
    // ROOK
    {
        32,  42,  32,  51, 63, 9,  31, 43,  27,  32,  58,  62,  80, 67, 26,  44,
        -5,  19,  26,  36, 17, 45, 61, 16,  -24, -11, 7,   26,  24, 35, -8,  -20,
        -36, -26, -12, -1, 9,  -7, 6,  -23, -45, -25, -16, -17, 3,  0,  -5,  -33,
        -44, -16, -20, -9, -1, 11, -6, -71, -19, -13, 1,   17,  16, 7,  -37, -26,
    },
    // QUEEN
    {
        -28, 0,   29, 12,  59, 44, 43, 45, -24, -39, -5,  1,   -16, 57,  28,  54,
        -13, -17, 7,  8,   29, 56, 47, 57, -27, -27, -16, -16, -1,  17,  -2,  1,
        -9,  -26, -9, -10, -2, -4, 3,  -3, -14, 2,   -11, -2,  -5,  2,   14,  5,
        -35, -8,  11, 2,   8,  15, -3, 1,  -1,  -18, -9,  -10, -30, -35, -18, -54,
    },
    // KING (Middlegame: Encourages safety)
    {
        -65, 23, 16,  -15, -35, -34, 2,   13,  29,  -1,  -20, -7,  -8,  -4,  -38, -29,
        -9,  24, 2,   -16, -20, 6,   22,  -22, -17, -20, -12, -27, -30, -25, -14, -36,
        -49, -1, -27, -39, -46, -44, -33, -51, -14, -14, -22, -46, -44, -30, -15, -27,
        1,   7,  -8,  -64, -43, -16, 9,   8,   -15, 36,  12,  -54, 8,   -28, 24,  14,
    }};
