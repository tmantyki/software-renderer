#ifndef RASTERIZER_HPP
#define RASTERIZER_HPP

#include <SDL2/SDL.h>

#include "server/game_state.hpp"
#include "ui/ui.hpp"

class Rasterizer {
 public:
  virtual void RasterizeGameState(const GameState& game_state,
                                  UserInterface& user_interface) noexcept = 0;
};

class WireframeRasterizer : public Rasterizer {
 public:
  virtual void RasterizeGameState(
      const GameState& game_state,
      UserInterface& user_interface) noexcept override;
};

#endif