#pragma once

#include <SDL2/SDL.h>

#include "geometry/common.hpp"
#include "geometry/texture.hpp"
#include "rasterizer/render_buffer.hpp"
#include "server/game_state.hpp"

struct OrderedVertexIndices {
  size_t top;
  size_t mid;
  size_t low;
};

struct PixelCoordinates {
  uint16_t top_x;
  uint16_t top_y;
  float top_z;
  uint16_t mid_x;
  uint16_t mid_y;
  float mid_z;
  uint16_t low_x;
  uint16_t low_y;
  float low_z;
};

struct InterpolationParameters {
  float final_z;
  float horizontal_t;
  float top_mid_t;
  float top_mid_z;
  float top_low_t;
  float top_low_z;
};

struct ScanlineParameters {
  uint16_t scan_x_left;
  uint16_t scan_x_right;
  int8_t scan_y_increment;
};

struct RasterizationContext {
  const GameState& game_state;
  RenderBuffer& render_buffer;
  const Texture& default_texture;
};

template <typename RasterPolicy, typename FillPolicy, typename ZBufferPolicy>
class Rasterizer {
 public:
  void RasterizeGameState(RasterizationContext& context) noexcept;
};

struct None {
  static void SetupPixels(RenderBuffer&) noexcept {}
  static void SetupZBuffer(RenderBuffer&) noexcept {}
};

struct ResetZBuffer {
  static void SetupZBuffer(RenderBuffer& render_buffer) noexcept;
};

struct BackgroundFill {
  static void SetupPixels(RenderBuffer& render_buffer) noexcept;
};

// struct WireframeRaster {
//   static void RasterizeTriangles(RasterizationContext& context) noexcept;

//  private:
//   static void DrawLine(f32 a_x,
//                        f32 a_y,
//                        f32 b_x,
//                        f32 b_y,
//                        RenderBuffer& render_buffer) noexcept;
// };

// struct FlatRaster {
//   static void RasterizeTriangles(RasterizationContext& context) noexcept;

//  private:
//   static void RasterizeTriangleHalf(PixelCoordinates& pc,
//                                     OrderedVertexIndices& vi,
//                                     TriangleHalf triangle_half,
//                                     f32 brightness,
//                                     RenderBuffer& render_buffer) noexcept;
// };

// struct TexturedRaster {
//   static void RasterizeTriangles(RasterizationContext& context) noexcept;

//  private:
//   static void RasterizeTriangleHalf(PixelCoordinates& pc,
//                                     OrderedVertexIndices& vi,
//                                     const Triangle& triangle,
//                                     TriangleHalf triangle_half,
//                                     f32 brightness,
//                                     RasterizationContext& context) noexcept;
// };

class PureRasterizer {
 public:
  void RasterizeGameState(RasterizationContext& context) noexcept;

 private:
  void RasterizeTriangleHalf(PixelCoordinates& pc,
                             OrderedVertexIndices& vi,
                             const TriangleSharedPointer& triangle,
                             TriangleHalf triangle_half,
                             f32 brightness,
                             const Texture* __restrict__ default_texture,
                             Sample* const __restrict__ pixels,
                             int pitch,
                             f32* const __restrict__ z_buffer) noexcept;
  void RasterizeTriangles(RasterizationContext* __restrict__ context) noexcept;
};

// using WireframeRasterizer = Rasterizer<WireframeRaster, BackgroundFill,
// None>; template class Rasterizer<WireframeRaster, BackgroundFill, None>;
// using FlatRasterizer = Rasterizer<FlatRaster, BackgroundFill, ResetZBuffer>;
// template class Rasterizer<FlatRaster, BackgroundFill, ResetZBuffer>;
// using TexturedRasterizer =
//     Rasterizer<TexturedRaster, BackgroundFill, ResetZBuffer>;
// template class Rasterizer<TexturedRaster, BackgroundFill, ResetZBuffer>;
