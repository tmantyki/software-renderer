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
  f32 top_x;
  f32 top_y;
  f32 top_z;
  f32 mid_x;
  f32 mid_y;
  f32 mid_z;
  f32 low_x;
  f32 low_y;
  f32 low_z;
};

struct InterpolationParameters {
  f32 final_z;
  f32 horizontal_t;
  f32 top_mid_t;
  f32 top_mid_z;
  f32 top_low_t;
  f32 top_low_z;
};

struct ScanlineParameters {
  f32 scan_x_left_f32;
  f32 scan_x_right_f32;
  uint16_t scan_x_left_u16;
  uint16_t scan_x_right_u16;
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

struct WireframeRaster {
  static void RasterizeTriangles(RasterizationContext& context) noexcept;

 private:
  static void DrawLine(f32 a_x,
                       f32 a_y,
                       f32 b_x,
                       f32 b_y,
                       RenderBuffer& render_buffer) noexcept;
};

struct FlatRaster {
  static void RasterizeTriangles(RasterizationContext& context) noexcept;

 private:
  static void RasterizeTriangleHalf(PixelCoordinates& pc,
                                    OrderedVertexIndices& vi,
                                    TriangleHalf triangle_half,
                                    f32 brightness,
                                    RenderBuffer& render_buffer) noexcept;
};

struct TexturedRaster {
  static void RasterizeTriangles(RasterizationContext& context) noexcept;

 private:
  static void RasterizeTriangleHalf(PixelCoordinates& pc,
                                    OrderedVertexIndices& vi,
                                    const Triangle& triangle,
                                    TriangleHalf triangle_half,
                                    f32 brightness,
                                    RasterizationContext& context) noexcept;
};

using WireframeRasterizer = Rasterizer<WireframeRaster, BackgroundFill, None>;
template class Rasterizer<WireframeRaster, BackgroundFill, None>;
using FlatRasterizer = Rasterizer<FlatRaster, BackgroundFill, ResetZBuffer>;
template class Rasterizer<FlatRaster, BackgroundFill, ResetZBuffer>;
using TexturedRasterizer =
    Rasterizer<TexturedRaster, BackgroundFill, ResetZBuffer>;
template class Rasterizer<TexturedRaster, BackgroundFill, ResetZBuffer>;
