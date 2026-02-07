#ifndef GAME_HPP
#define GAME_HPP

#include "engine.hpp"
#include "position.hpp"
#include "types.hpp"

class Game {
 private:
 public:
  Position position;
  Engine engine;

  Game();
  ~Game();
};

#endif
