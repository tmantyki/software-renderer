#ifndef RASTERIZER_HPP
#define RASTERIZER_HPP

#include <SDL2/SDL.h>

#include "geometry/direction.hpp"
#include "geometry/texture.hpp"
#include "server/game_state.hpp"
#include "ui/ui.hpp"

typedef struct PixelCoordinates {
  uint16_t top_x;
  uint16_t top_y;
  float top_z;
  uint16_t mid_x;
  uint16_t mid_y;
  float mid_z;
  uint16_t low_x;
  uint16_t low_y;
  float low_z;
} PixelCoordinates;

typedef struct InterpolationParameters {
  float final_z;
  float horizontal_t;
  float top_mid_t;
  float top_mid_z;
  float top_low_t;
  float top_low_z;
} InterpolationParameters;

typedef struct ScanlineParameters {
  uint16_t scan_x;
  uint16_t scan_y;
  uint16_t scan_x_left;
  uint16_t scan_x_right;
  int8_t scan_x_increment;
  int8_t scan_y_increment;
} ScanlineParameters;

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
  ScanlineRasterizer() noexcept;
  virtual void RasterizeGameState(
      const GameState& game_state,
      UserInterface& user_interface) noexcept override;

 protected:
  void ResetZBuffer() noexcept;
  void ClearRenderer() noexcept;
  bool ZBufferCheckAndReplace(float new_value,
                              uint32_t z_buffer_index) noexcept;
  std::array<float, kWindowWidth * kWindowHeight> z_buffer_;
  uint8_t* pixels_;
  int pitch_;

 private:
  void CalculateTrianglePixelCoordinates(PixelCoordinates& pc,
                                         const Space& space,
                                         size_t triangle_index) const noexcept;
  virtual void RasterizeTriangleHalf(size_t triangle_index,
                                     PixelCoordinates& pc,
                                     TriangleHalf triangle_half,
                                     uint8_t color_value) noexcept;
  void WritePixel(uint8_t color_value,
                  const ScanlineParameters& sp,
                  uint8_t* pixels,
                  int pitch) noexcept;
};

class FlatRasterizer : public ScanlineRasterizer {
 public:
  FlatRasterizer() noexcept;
  FlatRasterizer(Direction light_direction) noexcept;

 private:
  Direction light_direction_;
};

class TexturedRasterizer : public ScanlineRasterizer {
 public:
  TexturedRasterizer() noexcept;

 private:
  virtual void RasterizeTriangleHalf(size_t triangle_index,
                                     PixelCoordinates& pc,
                                     TriangleHalf triangle_half,
                                     uint8_t color_value) noexcept override;
  void WritePixel(const ScanlineParameters& sp,
                  uint8_t* pixels,
                  int pitch) noexcept;

  Texture texture_;
};

#endif