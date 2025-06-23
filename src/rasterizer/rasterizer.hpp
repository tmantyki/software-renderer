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

template <size_t buffer_length>
struct PixelMultiply {
  PixelMultiply() = delete;

  struct alignas(kCacheLineSize) Context {
    std::array<Pixel, buffer_length>& texels;
    std::array<u32, buffer_length>& pixel_offsets;
    const f32 brightness;
    size_t& counter;
    Pixel* const pixels;
  };

  static size_t Enqueue(Pixel pixel,
                        u32 pixel_offset,
                        Context& context) noexcept {
    context.texels[context.counter] = pixel;
    context.pixel_offsets[context.counter++] = pixel_offset;
    if (context.counter == buffer_length)
      FlushVectorized(context);
    return context.counter;
  }

  /*   void FlushSequential(f32 brightness) noexcept {
      for (size_t i = 0; i < counter_; i++) {
        u32 texel = texels_[i];
        texel.argb.alpha *= brightness;
        texel.argb.red *= brightness;
        texel.argb.green *= brightness;
        texel.argb.blue *= brightness;
        texels_[i] = texel;
      }
      counter_ = 0;
    } */

  static void FlushVectorized(Context& context) noexcept {
    constexpr size_t kBytesIn256Bits = 256 / 8;
    size_t constexpr kPixelsPerAVX256 =
        kBytesIn256Bits / (sizeof(f32) * kBytesPerPixel);
    static_assert(buffer_length % kPixelsPerAVX256 == 0);

    for (size_t i = 0; i < buffer_length; i += kPixelsPerAVX256) {
      void* texels_data = context.texels.data() + i;
      __m128i u8_values =
          _mm_loadl_epi64(reinterpret_cast<__m128i*>(texels_data));
      __m256i packed_32 = _mm256_cvtepu8_epi32((u8_values));
      __m256 f32_values = _mm256_cvtepi32_ps(packed_32);
      __m256 brightness_vector = _mm256_set1_ps(context.brightness);
      f32_values = _mm256_mul_ps(f32_values, brightness_vector);
      packed_32 = _mm256_cvtps_epi32(f32_values);
      __m256i mask = _mm256_setr_epi8(0, 4, 8, 12, 16, 20, 24, 28, -1, -1, -1,
                                      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                      -1, -1, -1, 0, 4, 8, 12, 16, 20, 24, 28);
      packed_32 = _mm256_shuffle_epi8(packed_32, mask);

      mask = _mm256_setr_epi32(0, 6, -1, -1, -1, -1, -1, -1);
      packed_32 = _mm256_permutevar8x32_epi32(packed_32, mask);

      _mm_storel_epi64(reinterpret_cast<__m128i*>(texels_data),
                       _mm256_castsi256_si128(packed_32));
      context.pixels[context.pixel_offsets[i]] = context.texels[i];
      context.pixels[context.pixel_offsets[i + 1]] = context.texels[i + 1];
    }
    context.counter = 0;
  }
};

using WireframeRasterizer = Rasterizer<WireframeRaster, BackgroundFill, None>;
template class Rasterizer<WireframeRaster, BackgroundFill, None>;
using FlatRasterizer = Rasterizer<FlatRaster, BackgroundFill, ResetZBuffer>;
template class Rasterizer<FlatRaster, BackgroundFill, ResetZBuffer>;
using TexturedRasterizer =
    Rasterizer<TexturedRaster, BackgroundFill, ResetZBuffer>;
template class Rasterizer<TexturedRaster, BackgroundFill, ResetZBuffer>;
