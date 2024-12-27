#ifndef RASTERIZER_HPP
#define RASTERIZER_HPP

#include <SDL2/SDL.h>

#include "geometry/direction.hpp"
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

class FlatRasterizer : public Rasterizer {
 public:
  FlatRasterizer() noexcept;
  FlatRasterizer(Direction light_direction) noexcept;
  virtual void RasterizeGameState(
      const GameState& game_state,
      UserInterface& user_interface) noexcept override;

 private:
  Direction light_direction_;
  std::array<std::array<float,800>,800> z_buffer_;
};

#endif