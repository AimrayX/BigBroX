#ifndef POSITION_HPP
#define POSITION_HPP

#include <stdint.h>

#include <cstdint>
#include <string>

#include "types.hpp"

class Position {
 private:
 public:
  uint32_t mNumberOfWhitePieces;
  uint32_t mNumberOfBlackPieces;

  Color mSideToMove;
  int mCastleRight;

  uint64_t mEnPassentSquare;
  int mHalfMove;
  int mFullMove;

  uint64_t pieces[2][6];
  uint64_t occupancies[3];
  int board[64];

  uint64_t mHash;

  uint64_t getHash() const { return mHash; }

  int gamePly = 0;

  Eval posEval;
  uint64_t mAttacksP = 0ULL;

  // Full castling mask array
  // Indices: 0=a1, 7=h1, 56=a8, 63=h8
  static constexpr int castling_rights[64] = {
      // Rank 1 (White)
      ~WHITE_OOO, -1, -1, -1, ~(WHITE_OO | WHITE_OOO), -1, -1, ~WHITE_OO,
      // Rank 2-7 (Empty - no castling rights change)
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,
      // Rank 8 (Black)
      ~BLACK_OOO, -1, -1, -1, ~(BLACK_OO | BLACK_OOO), -1, -1, ~BLACK_OO};

  std::string mStartingPosition;
  StateInfo history[5120];

  uint64_t attackGeneration(int square, int type, Color color);
  uint64_t getPseudoLegalMoves(int piece, int type, Color color);
  int setStartingPosition(std::string startingPosition);
  int getPieceValue(int piece, int square, Color color);
  void doMove(Move m);
  void undoMove(Move m);
  void doNullMove();
  void undoNullMove();
  void getMoves(Color color, MoveList& moveList);
  void getCaptures(Color color, MoveList& moveList);
  bool isSquareAttacked(int square, Color sideAttacking);
  bool isRepetition();
  bool isCheck();
  bool hasNonPawnMaterial(Color side) const;
  inline void addPawnCaptureMove(int from, int to, MoveList& moveList);
  uint64_t predictChildHash(Move m);

  void printBoard();

  Position();
  ~Position();
};

#endif
