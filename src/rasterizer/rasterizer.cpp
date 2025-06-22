#include <SDL2/SDL.h>

#include <algorithm>
#include <utility>
#include "geometry/common.hpp"
#include "geometry/space.hpp"
#include "rasterizer/rasterizer.hpp"
#include "rasterizer/render_buffer.hpp"

#include <emmintrin.h>
#include <immintrin.h>

namespace {

[[nodiscard]] OrderedVertexIndices GetYSortedVertexIndices(
    size_t triangle_index,
    const Space& space) noexcept {
  const VertexMatrix& vertices = space.GetVertices();
  size_t a = triangle_index * kVerticesPerTriangle;
  size_t b = triangle_index * kVerticesPerTriangle + 1;
  size_t c = triangle_index * kVerticesPerTriangle + 2;
  f32 ay = vertices(kY, a);
  f32 by = vertices(kY, b);
  f32 cy = vertices(kY, c);

  // Sort three elements
  if (ay > cy) {
    std::swap(ay, cy);
    std::swap(a, c);
  }
  if (ay > by) {
    std::swap(ay, by);
    std::swap(a, b);
  }
  if (by > cy) {
    std::swap(by, cy);
    std::swap(b, c);
  }
  return {a, b, c};
}

// #TODO refcator with arrays rather than top, mid, low etc.
[[nodiscard]] PixelCoordinates GetPixelCoordinates(
    const OrderedVertexIndices& vertex_indices,
    const Space& space) noexcept {
  const VertexMatrix& vertices = space.GetVertices();
  PixelCoordinates ret_pc;
  ret_pc.top_x = vertices(kX, vertex_indices.top);
  ret_pc.top_y = vertices(kY, vertex_indices.top);
  ret_pc.top_z = vertices(kZ, vertex_indices.top);
  ret_pc.mid_x = vertices(kX, vertex_indices.mid);
  ret_pc.mid_y = vertices(kY, vertex_indices.mid);
  ret_pc.mid_z = vertices(kZ, vertex_indices.mid);
  ret_pc.low_x = vertices(kX, vertex_indices.low);
  ret_pc.low_y = vertices(kY, vertex_indices.low);
  ret_pc.low_z = vertices(kZ, vertex_indices.low);
  return ret_pc;
}

void SwapTopAndLow(PixelCoordinates& pc, OrderedVertexIndices& vi) noexcept {
  std::swap(pc.low_x, pc.top_x);
  std::swap(pc.low_y, pc.top_y);
  std::swap(pc.low_z, pc.top_z);
  std::swap(vi.low, vi.top);
}

void CalculateXScanlineBoundaries(ScanlineParameters& sp,
                                  u16 scan_y,
                                  const PixelCoordinates pc,
                                  bool& left_right_swapped) noexcept {
  sp.scan_x_left_f32 = (scan_y * pc.low_x - pc.top_x * scan_y -
                        pc.top_y * pc.low_x + pc.top_x * pc.low_y) /
                       (pc.low_y - pc.top_y);
  sp.scan_x_right_f32 = (scan_y * pc.mid_x - pc.top_x * scan_y -
                         pc.top_y * pc.mid_x + pc.top_x * pc.mid_y) /
                        (pc.mid_y - pc.top_y);
  if (sp.scan_x_right_f32 < sp.scan_x_left_f32) {
    std::swap(sp.scan_x_left_f32, sp.scan_x_right_f32);
    left_right_swapped = true;
  } else
    left_right_swapped = false;
  sp.scan_x_left_u16 = std::floor(sp.scan_x_left_f32);
  sp.scan_x_right_u16 = std::floor(sp.scan_x_right_f32);  // Necessary floor?
  // sp.scan_x_increment = sp.scan_x_right < sp.scan_x_left ? -1 : 1;
}

void CalculateInterpolationParametersForY(InterpolationParameters& ip,
                                          const u16 scan_y,
                                          const PixelCoordinates& pc) noexcept {
  f32 numerator = static_cast<f32>(scan_y - pc.top_y);
  ip.top_low_t = numerator / (pc.low_y - pc.top_y);
  ip.top_mid_t = numerator / (pc.mid_y - pc.top_y);
  ip.top_low_z = pc.top_z * (1.0f - ip.top_low_t) + pc.low_z * ip.top_low_t;
  ip.top_mid_z = pc.top_z * (1.0f - ip.top_mid_t) + pc.mid_z * ip.top_mid_t;
}

void CalculateInterpolationParametersForX(
    InterpolationParameters& ip,
    const ScanlineParameters& sp,
    const u16 scan_x,
    const bool left_right_swapped) noexcept {
  ip.horizontal_t = static_cast<f32>(scan_x - sp.scan_x_left_f32) /
                    (sp.scan_x_right_f32 - sp.scan_x_left_f32);
  if (left_right_swapped)
    ip.horizontal_t = 1.0f - ip.horizontal_t;
  ip.final_z =
      ip.top_low_z * (1.0f - ip.horizontal_t) + ip.top_mid_z * ip.horizontal_t;
}

// #TODO: citation?
f32 TrueZ(f32 reciprocal_z) noexcept {
  constexpr f32 A = 2.0f * kNearPlaneDistance * kFarPlaneDistance /
                    (kFarPlaneDistance - kNearPlaneDistance);
  constexpr f32 B = (kFarPlaneDistance + kNearPlaneDistance) /
                    (kFarPlaneDistance - kNearPlaneDistance);
  return A / (reciprocal_z - B);
}

bool ZBufferCheckAndReplace(float new_value,
                            u32 z_buffer_index,
                            f32* const z_buffer) noexcept {
  // if (new_value - 0.001f < z_buffer[z_buffer_index]) {
  if (new_value < z_buffer[z_buffer_index]) {
    z_buffer[z_buffer_index] = new_value;
    return true;
  } else
    return false;
}
}  // namespace

// Rasterizer template

template <typename RasterPolicy,
          typename PixelSetupPolicy,
          typename ZBufferSetupPolicy>
void Rasterizer<RasterPolicy, PixelSetupPolicy, ZBufferSetupPolicy>::
    RasterizeGameState(RasterizationContext& context) noexcept {
  PixelSetupPolicy::SetupPixels(context.render_buffer);
  ZBufferSetupPolicy::SetupZBuffer(context.render_buffer);
  RasterPolicy::RasterizeTriangles(context);
}

// Generic policies

void BackgroundFill::SetupPixels(RenderBuffer& render_buffer) noexcept {
  const int pitch = render_buffer.pitch;
  u8* const pixels = reinterpret_cast<u8*>(render_buffer.pixels);
  // #TODO: rid of magic number
  __m256i write_vec = _mm256_set1_epi32(0xff008080U);
  for (i32 i = 0; i < kWindowHeight * pitch; i += 32) {
    _mm256_store_si256(reinterpret_cast<__m256i*>(pixels + i), write_vec);
  }
  // #TODO : remainder handling
  for (i32 i = 0; i < kWindowHeight * kWindowWidth * 4; i += 4) {
    assert(*reinterpret_cast<u32*>(pixels + i) == 0xff008080U);
  }
}

void ResetZBuffer::SetupZBuffer(RenderBuffer& render_buffer) noexcept {
  f32* const z_buffer = render_buffer.z_buffer;
  __m256 write_vec = _mm256_set1_ps(kZBufferMax);
  for (i32 i = 0; i < kWindowHeight * kWindowWidth; i += 8) {
    _mm256_store_ps(reinterpret_cast<float*>(z_buffer + i), write_vec);
  }
  // #TODO : remainder handling

  for (i32 i = 0; i < kWindowHeight * kWindowWidth; i++) {
    assert(z_buffer[i] == kZBufferMax);
  }
}

// /************************
//  * Wireframe rasterizer *
//  ***********************/

void WireframeRaster::RasterizeTriangles(
    RasterizationContext& context) noexcept {
  const Space& space = context.game_state.GetOutputSpace();
  const VertexMatrix& vertices = space.GetVertices();
  for (size_t t = 0; t < space.GetTriangleCount(); t++) {
    for (size_t a = 0; a < kVerticesPerTriangle; a++) {
      size_t b = (a + 1) % 3;
      f32 a_x = vertices(kX, t * kVerticesPerTriangle + a);
      f32 a_y = vertices(kY, t * kVerticesPerTriangle + a);
      f32 b_x = vertices(kX, t * kVerticesPerTriangle + b);
      f32 b_y = vertices(kY, t * kVerticesPerTriangle + b);
      DrawLine(a_x, a_y, b_x, b_y, context.render_buffer);
    }
  }
}

// #TODO: rewrite and fix artifacts
void WireframeRaster::DrawLine(f32 a_x,
                               f32 a_y,
                               f32 b_x,
                               f32 b_y,
                               RenderBuffer& render_buffer) noexcept {
  const Sample fg_color = {{0xff, 0xff, 0xff, 0xff}};
  const bool swap_xy = std::abs(b_x - a_x) < std::abs(b_y - a_y);
  if (!swap_xy) {
    f32 increment = (b_y - a_y) / (b_x - a_x);
    f32 y = a_y + 0.5;
    for (u16 x = a_x; x <= b_x; x++, y += increment)
      render_buffer.pixels[x + static_cast<u16>(y) * render_buffer.pitch /
                                   kBytesPerPixel] = fg_color;
  } else {
    f32 increment = (b_x - a_x) / (b_y - a_y);
    f32 x = a_x + 0.5;
    for (u16 y = a_y; y <= b_y; y++, x += increment)
      render_buffer.pixels[static_cast<u16>(x) +
                           y * render_buffer.pitch / kBytesPerPixel] = fg_color;
  }
}

// /*******************
//  * Flat rasterizer *
//  ******************/

void FlatRaster::RasterizeTriangles(RasterizationContext& context) noexcept {
  const Space& space = context.game_state.GetOutputSpace();
  for (size_t t = 0; t < space.GetTriangleCount(); t++) {
    const Triangle& triangle = *space.GetTriangles()[t];
    // Triangle color
    Direction light_direction = {1, 2, -3};
    f32 brightness = light_direction.GetVector().normalized().dot(
        triangle.GetNormal().normalized());
    brightness = (brightness + 1.0f) / 2.0f;

    OrderedVertexIndices vi = ::GetYSortedVertexIndices(t, space);
    PixelCoordinates pc = ::GetPixelCoordinates(vi, space);

    // Scanlines for top section
    RasterizeTriangleHalf(pc, vi, TriangleHalf::kUpper, brightness,
                          context.render_buffer);
    // Scanlines for bottom section
    RasterizeTriangleHalf(pc, vi, TriangleHalf::kLower, brightness,
                          context.render_buffer);
  }
}

void FlatRaster::RasterizeTriangleHalf(PixelCoordinates& pc,
                                       OrderedVertexIndices& vi,
                                       TriangleHalf triangle_half,
                                       f32 brightness,
                                       RenderBuffer& render_buffer) noexcept {
  (void)pc;
  (void)vi;
  (void)triangle_half;
  (void)brightness;
  (void)render_buffer;

  /* Sample* const pixels = render_buffer.pixels;
  const int pitch = render_buffer.pitch;
  InterpolationParameters ip;
  ScanlineParameters sp;
  bool left_right_swapped;
  u8 channel_value = 0xff * brightness;
  Sample argb_value = {{channel_value, channel_value, channel_value, 0xff}};

  if (triangle_half == TriangleHalf::kLower)
    ::SwapTopAndLow(pc, vi);

  if (pc.low_y == pc.top_y || pc.top_y == pc.mid_y)  // #TODO: Best timing?
    return;

  ::SetScanlineIncrementY(sp, triangle_half);
  for (u16 scan_y = pc.top_y;; scan_y += sp.scan_y_increment) {
    assert(scan_y < kWindowHeight);
    ::CalculateXScanlineBoundaries(sp, scan_y, pc, left_right_swapped);
    ::CalculateInterpolationParametersForY(ip, scan_y, pc);
    for (u16 scan_x = sp.scan_x_left; scan_x <= sp.scan_x_right; scan_x++) {
      assert(scan_x < kWindowWidth);
      ::CalculateInterpolationParametersForX(ip, sp, scan_x,
                                             left_right_swapped);
      if (::ZBufferCheckAndReplace(ip.final_z, scan_y * kWindowWidth + scan_x,
                                   render_buffer.z_buffer)) {
        size_t index = scan_y * (pitch / kBytesPerPixel) + scan_x;
        pixels[index] = argb_value;
      }
      if (scan_x == sp.scan_x_right)
        break;
    }
    if (scan_y == pc.mid_y)
      break;
  } */
}

/***********************
 * Textured rasterizer *
 **********************/

void TexturedRaster::RasterizeTriangles(
    RasterizationContext& context) noexcept {
  const Space& space = context.game_state.GetOutputSpace();
  for (size_t t = 0; t < space.GetTriangleCount(); t++) {
    const Triangle& triangle = *space.GetTriangles()[t];
    // Triangle color
    Direction light_direction = {1, 2, -3};
    f32 brightness = light_direction.GetVector().normalized().dot(
        triangle.GetNormal().normalized());
    brightness = (brightness + 1.0f) / 2.0f;

    OrderedVertexIndices vi = ::GetYSortedVertexIndices(t, space);
    PixelCoordinates pc = ::GetPixelCoordinates(vi, space);

    // Scanlines for top section
    RasterizeTriangleHalf(pc, vi, triangle, TriangleHalf::kUpper, brightness,
                          context);
    // Scanlines for bottom section
    RasterizeTriangleHalf(pc, vi, triangle, TriangleHalf::kLower, brightness,
                          context);
  }
}

void TexturedRaster::RasterizeTriangleHalf(
    PixelCoordinates& pc,
    OrderedVertexIndices& vi,
    const Triangle& triangle,
    TriangleHalf triangle_half,
    f32 brightness,
    RasterizationContext& context) noexcept {
  RenderBuffer& render_buffer = context.render_buffer;
  const Texture& default_texture = context.default_texture;
  Sample* const pixels = render_buffer.pixels;
  const int pitch = render_buffer.pitch;
  InterpolationParameters ip;
  ScanlineParameters sp;
  bool left_right_swapped;

  const u16 texture_width = default_texture.GetWidth();
  const u16 texture_height = default_texture.GetHeight();
  const SDL_Surface* texture_surface = default_texture.GetSurface();
  const int texture_pitch = texture_surface->pitch;

  // Used by SIMD-intrinsics

  constexpr size_t buffer_length = 2;
  alignas(64) std::array<u32, buffer_length> texels;
  alignas(64) std::array<u32, buffer_length> pixel_offsets;
  size_t counter = 0;
  SampleMultiply<buffer_length>::Context sample_multiply_context = {
      texels, pixel_offsets, brightness, counter,
      reinterpret_cast<u32*>(pixels)};

  // SIMD_context<2> simd_context;

  if (triangle_half == TriangleHalf::kLower)
    ::SwapTopAndLow(pc, vi);

  const UVCoordinate& top_uv =
      triangle.GetVertex(vi.top % kVerticesPerTriangle).GetUVCoordinate();
  const UVCoordinate& mid_uv =
      triangle.GetVertex(vi.mid % kVerticesPerTriangle).GetUVCoordinate();
  const UVCoordinate& low_uv =
      triangle.GetVertex(vi.low % kVerticesPerTriangle).GetUVCoordinate();

  i16 y1 = std::ceil(std::min(pc.top_y, pc.mid_y));
  i16 y2 = std::floor(std::max(pc.top_y, pc.mid_y));
  if (triangle_half == kLower)
    std::swap(y1, y2);

  for (i16 scan_y = y1; scan_y != y2 + triangle_half; scan_y += triangle_half) {
    assert(scan_y < kWindowHeight);
    ::CalculateXScanlineBoundaries(sp, scan_y, pc, left_right_swapped);
    ::CalculateInterpolationParametersForY(ip, scan_y, pc);

    Vector2 top_low_uv = (top_uv / ::TrueZ(pc.top_z)) * (1.0f - ip.top_low_t) +
                         (low_uv / ::TrueZ(pc.low_z)) * (ip.top_low_t);
    top_low_uv *= ::TrueZ(ip.top_low_z);

    Vector2 top_mid_uv = (top_uv / ::TrueZ(pc.top_z)) * (1.0f - ip.top_mid_t) +
                         (mid_uv / ::TrueZ(pc.mid_z)) * (ip.top_mid_t);
    top_mid_uv *= ::TrueZ(ip.top_mid_z);

    for (u16 scan_x = sp.scan_x_left_u16; scan_x < sp.scan_x_right_u16;
         scan_x++) {
      assert(scan_x < kWindowWidth);
      ::CalculateInterpolationParametersForX(ip, sp, scan_x,
                                             left_right_swapped);

      Vector2 final_uv =
          (top_low_uv / ::TrueZ(ip.top_low_z)) * (1 - ip.horizontal_t) +
          (top_mid_uv / ::TrueZ(ip.top_mid_z)) * (ip.horizontal_t);
      final_uv *= ::TrueZ(ip.final_z);

      // #TODO: replace argument with incrementing
      if (::ZBufferCheckAndReplace(ip.final_z, scan_y * kWindowWidth + scan_x,
                                   render_buffer.z_buffer)) {
        u16 u = static_cast<u16>(final_uv[kU] * (texture_width));
        u16 v = static_cast<u16>((1 - final_uv[kV]) * (texture_height));

        // CLAMP #TODO: best clamping strategy? (-1 case)
        u = std::min(texture_width - 1, static_cast<i32>(u));
        v = std::min(texture_height - 1, static_cast<i32>(v));

        assert(u < texture_width);
        assert(v < texture_height);
        // u32* target_pixels = reinterpret_cast<u32*>(pixels);
        u32* texture_pixels = reinterpret_cast<u32*>(texture_surface->pixels);
        u32 target_offset = scan_y * (pitch / kBytesPerPixel) + scan_x;
        u32 texture_offset = v * (texture_pitch / kBytesPerPixel) + u;

        SampleMultiply<buffer_length>::Enqueue(texture_pixels[texture_offset],
                                               target_offset,
                                               sample_multiply_context);
      }
    }
  }
}
