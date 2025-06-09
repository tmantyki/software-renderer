#pragma once

#include <SDL2/SDL.h>

#include "geometry/common.hpp"
#include "geometry/direction.hpp"
#include "geometry/texture.hpp"
#include "server/game_state.hpp"
#include "ui/ui.hpp"

typedef struct OrderedVertexIndices {
  size_t top;
  size_t mid;
  size_t low;
} OrderedVertexIndices;

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
  uint16_t scan_x_left;
  uint16_t scan_x_right;
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
  ScanlineRasterizer() noexcept {}
  virtual void RasterizeGameState(
      const GameState& game_state,
      UserInterface& user_interface) noexcept override;

 protected:
  void ResetZBuffer() noexcept {
    std::fill(z_buffer_.begin(), z_buffer_.end(), 1);
  }
  void ClearRenderer() noexcept;
  bool ZBufferCheckAndReplace(float new_value,
                              uint32_t z_buffer_index) noexcept {
    if (new_value - 0.001f < z_buffer_[z_buffer_index]) {
      z_buffer_[z_buffer_index] = new_value;
      return true;
    } else
      return false;
  }
  std::array<float, kWindowWidth * kWindowHeight> z_buffer_;
  uint8_t* pixels_;
  int pitch_;

 private:
  void SetSortedVertexIndices(OrderedVertexIndices& vi,
                              const size_t triangle_index,
                              const Space& space) const noexcept;
  void SetPixelCoordinates(PixelCoordinates& pc,
                           const OrderedVertexIndices& vertex_indices,
                           const Space& space) const noexcept;
  virtual void RasterizeTriangleHalf(PixelCoordinates& pc,
                                     OrderedVertexIndices& vi,
                                     const TriangleSharedPointer& triangle,
                                     TriangleHalf triangle_half,
                                     float brightness) noexcept;
  // void WritePixel(uint8_t color_value,
  //                 const ScanlineParameters& sp,
  //                 uint8_t* pixels,
  //                 int pitch) noexcept;
};

class FlatRasterizer : public ScanlineRasterizer {
 public:
  FlatRasterizer() noexcept : FlatRasterizer({1, 1, 1}) {}
  FlatRasterizer(Direction light_direction) noexcept
      : light_direction_(light_direction) {}

 private:
  Direction light_direction_;
};

class TexturedRasterizer : public ScanlineRasterizer {
 public:
  // #TODO: exception handling?
  TexturedRasterizer() : texture_("assets/blender/porcelain.png") {}

 private:
  virtual void RasterizeTriangleHalf(PixelCoordinates& pc,
                                     OrderedVertexIndices& vi,
                                     const TriangleSharedPointer& triangle,
                                     TriangleHalf triangle_half,
                                     float brightness) noexcept override;
  // void WritePixel(const ScanlineParameters& sp,
  //                 uint8_t* pixels,
  //                 int pitch,
  //                 const uint16_t texture_width,
  //                 const uint16_t texture_height,
  //                 const SDL_Surface* texture_surface,
  //                 const int texture_pitch,
  //                 const Vector2& uv) noexcept;

  Texture texture_;
};
