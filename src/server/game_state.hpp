#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

#include "geometry/space.hpp"
#include "server/player.hpp"

class GameState {
 public:
  GameState();
  void ProcessTick();

 private:
  Space world_space_;
  Player player_;
  uint64_t tick_counter_;
};

#endif