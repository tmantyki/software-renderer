#include "game_state.hpp"

GameState::GameState() : tick_counter_(0) {}

void GameState::ProcessTick() {
  tick_counter_++;
}