#ifndef RASTERIZER_HPP
#define RASTERIZER_HPP

#include <SDL2/SDL.h>

#include "geometry/direction.hpp"
#include "server/game_state.hpp"
#include "ui/ui.hpp"

typedef struct RasterizerContext {
  const Space& space;
  SDL_Renderer* renderer;
  SDL_Texture* texture;
} RasterizerContext;

typedef struct PixelCoordinates {
  uint16_t apex_x;
  uint16_t apex_y;
  float apex_z;
  uint16_t mid_x;
  uint16_t mid_y;
  float mid_z;
  uint16_t base_x;
  uint16_t base_y;
  float base_z;
} PixelCoordinates;

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

class ScanlineRasterizer : public Rasterizer {
 public:
  ScanlineRasterizer() = delete;
  virtual void RasterizeGameState(
      const GameState& game_state,
      UserInterface& user_interface) noexcept override;

 private:
  void ResetZBuffer() noexcept;
  void ClearRenderer(uint8_t* pixels, int pitch) noexcept;
  void CalculateTrianglePixelCoordinates(PixelCoordinates& pixel_coordinates,
                                         const Space& space,
                                         size_t triangle_index) const noexcept;
  // void RasterizeTriangle(size_t triangle_index);
  void RasterizeTriangleHalf(size_t triangle_index,
                             PixelCoordinates& pixel_coordinates,
                             TriangleHalf triangle_half) noexcept;
  virtual void RasterizeTirangleScanline(uint16_t x, uint16_t y) = 0;
  std::array<std::array<float, 800>, 800> z_buffer_;
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
  std::array<std::array<float, 800>, 800> z_buffer_;
};

#endif