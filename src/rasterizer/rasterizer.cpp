#include <SDL2/SDL.h>

#include <algorithm>
#include "geometry/common.hpp"
#include "rasterizer/rasterizer.hpp"
#include "rasterizer/render_buffer.hpp"

#include <emmintrin.h>
#include <immintrin.h>

namespace {

size_t GetBoundaryVertexIndexByDimension(size_t a_index,
                                                size_t b_index,
                                                size_t c_index,
                                                enum Axis axis,
                                                BoundaryType boundary_type,
                                                const VertexMatrix& vertices) {
  f32 a = vertices(axis, a_index);
  f32 b = vertices(axis, b_index);
  f32 c = vertices(axis, c_index);
  f32 boundary_value;
  if (boundary_type == BoundaryType::kMin)
    boundary_value = std::min({a, b, c});
  else
    boundary_value = std::max({a, b, c});
  if (a == boundary_value)
    return a_index;
  else if (b == boundary_value)
    return b_index;
  else
    return c_index;
}

// #TODO: refactor with simpler sorting logic
void SetSortedVertexIndices(OrderedVertexIndices& vertex_indices,
                                   const size_t triangle_index,
                                   const Space& space) noexcept {
  size_t a_index = triangle_index * kVerticesPerTriangle;
  size_t b_index = triangle_index * kVerticesPerTriangle + 1;
  size_t c_index = triangle_index * kVerticesPerTriangle + 2;
  const VertexMatrix& vertices = space.GetVertices();
  vertex_indices.top = ::GetBoundaryVertexIndexByDimension(
      a_index, b_index, c_index, kY, BoundaryType::kMin, vertices);
  vertex_indices.low = ::GetBoundaryVertexIndexByDimension(
      a_index, b_index, c_index, kY, BoundaryType::kMax, vertices);
  if ((a_index != vertex_indices.top) && (a_index != vertex_indices.low))
    vertex_indices.mid = a_index;
  else if ((b_index != vertex_indices.top) && (b_index != vertex_indices.low))
    vertex_indices.mid = b_index;
  else
    vertex_indices.mid = c_index;
}

// #TODO refcator with arrays rather than top, mid, low etc.
void SetPixelCoordinates(PixelCoordinates& pc,
                                const OrderedVertexIndices& vertex_indices,
                                const Space& space) noexcept {
  const VertexMatrix& vertices = space.GetVertices();
  pc.top_x = vertices(kX, vertex_indices.top);
  pc.top_y = vertices(kY, vertex_indices.top);
  pc.top_z = vertices(kZ, vertex_indices.top);
  pc.mid_x = vertices(kX, vertex_indices.mid);
  pc.mid_y = vertices(kY, vertex_indices.mid);
  pc.mid_z = vertices(kZ, vertex_indices.mid);
  pc.low_x = vertices(kX, vertex_indices.low);
  pc.low_y = vertices(kY, vertex_indices.low);
  pc.low_z = vertices(kZ, vertex_indices.low);
}

void SwapTopAndLow(PixelCoordinates& pc,
                          OrderedVertexIndices& vi) noexcept {
  std::swap(pc.low_x, pc.top_x);
  std::swap(pc.low_y, pc.top_y);
  std::swap(pc.low_z, pc.top_z);
  std::swap(vi.low, vi.top);
}

void CalculateXScanlineBoundaries(ScanlineParameters& sp,
                                         u16 scan_y,
                                         const PixelCoordinates pc,
                                         bool& left_right_swapped) noexcept {
  sp.scan_x_left = (scan_y * pc.low_x - pc.top_x * scan_y -
                    pc.top_y * pc.low_x + pc.top_x * pc.low_y) /
                   (pc.low_y - pc.top_y);
  sp.scan_x_right = (scan_y * pc.mid_x - pc.top_x * scan_y -
                     pc.top_y * pc.mid_x + pc.top_x * pc.mid_y) /
                    (pc.mid_y - pc.top_y);
  if (sp.scan_x_right < sp.scan_x_left) {
    std::swap(sp.scan_x_left, sp.scan_x_right);
    left_right_swapped = true;
  } else
    left_right_swapped = false;
  // sp.scan_x_increment = sp.scan_x_right < sp.scan_x_left ? -1 : 1;
}

void CalculateInterpolationParametersForY(
    InterpolationParameters& ip,
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
  ip.horizontal_t = static_cast<f32>(scan_x - sp.scan_x_left) /
                    (sp.scan_x_right - sp.scan_x_left);
  if (left_right_swapped)
    ip.horizontal_t = 1.0f - ip.horizontal_t;
  ip.final_z =
      ip.top_low_z * (1.0f - ip.horizontal_t) + ip.top_mid_z * ip.horizontal_t;
}

void SetScanlineIncrementY(ScanlineParameters& sp,
                                  const TriangleHalf triangle_half) noexcept {
  sp.scan_y_increment = triangle_half == TriangleHalf::kLower ? -1 : 1;
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
                                   uint32_t z_buffer_index,
                                   f32* const __restrict__ z_buffer) noexcept {
  if (new_value - 0.001f < z_buffer[z_buffer_index]) {
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
  constexpr Sample bg_color = {{0x80, 0x80, 0x00, 0xff}};
  Sample* const pixels = render_buffer.pixels;
  const int pitch = render_buffer.pitch;
  std::fill(pixels, pixels + kWindowHeight * (pitch / kBytesPerPixel),
            bg_color);
}

// /************************
//  * Wireframe rasterizer *
//  ***********************/

// void WireframeRaster::RasterizeTriangles(
//     RasterizationContext& context) noexcept {
//   const Space& space = context.game_state.GetOutputSpace();
//   const VertexMatrix& vertices = space.GetVertices();
//   for (size_t t = 0; t < space.GetTriangleCount(); t++) {
//     for (size_t a = 0; a < kVerticesPerTriangle; a++) {
//       size_t b = (a + 1) % 3;
//       f32 a_x = vertices(kX, t * kVerticesPerTriangle + a);
//       f32 a_y = vertices(kY, t * kVerticesPerTriangle + a);
//       f32 b_x = vertices(kX, t * kVerticesPerTriangle + b);
//       f32 b_y = vertices(kY, t * kVerticesPerTriangle + b);
//       DrawLine(a_x, a_y, b_x, b_y, context.render_buffer);
//     }
//   }
// }

// // #TODO: rewrite and fix artifacts
// void WireframeRaster::DrawLine(f32 a_x,
//                                f32 a_y,
//                                f32 b_x,
//                                f32 b_y,
//                                RenderBuffer& render_buffer) noexcept {
//   const Sample fg_color = {{0xff, 0xff, 0xff, 0xff}};
//   const bool swap_xy = std::abs(b_x - a_x) < std::abs(b_y - a_y);
//   if (!swap_xy) {
//     f32 increment = (b_y - a_y) / (b_x - a_x);
//     f32 y = a_y + 0.5;
//     for (u16 x = a_x; x <= b_x; x++, y += increment)
//       render_buffer.pixels[x + static_cast<u16>(y) * render_buffer.pitch /
//                                    kBytesPerPixel] = fg_color;
//   } else {
//     f32 increment = (b_x - a_x) / (b_y - a_y);
//     f32 x = a_x + 0.5;
//     for (u16 y = a_y; y <= b_y; y++, x += increment)
//       render_buffer.pixels[static_cast<u16>(x) +
//                            y * render_buffer.pitch / kBytesPerPixel] =
//                            fg_color;
//   }
// }

// /*******************
//  * Flat rasterizer *
//  ******************/

// void FlatRaster::RasterizeTriangles(RasterizationContext& context) noexcept {
//   const Space& space = context.game_state.GetOutputSpace();
//   for (size_t t = 0; t < space.GetTriangleCount(); t++) {
//     const Triangle& triangle = *space.GetTriangles()[t];
//     // Triangle color
//     Direction light_direction = {1, 2, -3};
//     f32 brightness = light_direction.GetVector().normalized().dot(
//         triangle.GetNormal().normalized());
//     brightness = (brightness + 1.0f) / 2.0f;

//     PixelCoordinates pc;
//     OrderedVertexIndices vi;

//     ::SetSortedVertexIndices(vi, t, space);
//     ::SetPixelCoordinates(pc, vi, space);

//     // Scanlines for top section
//     RasterizeTriangleHalf(pc, vi, TriangleHalf::kUpper, brightness,
//                           context.render_buffer);
//     // Scanlines for bottom section
//     RasterizeTriangleHalf(pc, vi, TriangleHalf::kLower, brightness,
//                           context.render_buffer);
//   }
// }

// void FlatRaster::RasterizeTriangleHalf(PixelCoordinates& pc,
//                                        OrderedVertexIndices& vi,
//                                        TriangleHalf triangle_half,
//                                        f32 brightness,
//                                        RenderBuffer& render_buffer) noexcept
//                                        {
//   Sample* const pixels = render_buffer.pixels;
//   const int pitch = render_buffer.pitch;
//   InterpolationParameters ip;
//   ScanlineParameters sp;
//   bool left_right_swapped;
//   u8 channel_value = 0xff * brightness;
//   Sample argb_value = {{channel_value, channel_value, channel_value, 0xff}};

//   if (triangle_half == TriangleHalf::kLower)
//     ::SwapTopAndLow(pc, vi);

//   if (pc.low_y == pc.top_y || pc.top_y == pc.mid_y)  // #TODO: Best timing?
//     return;

//   ::SetScanlineIncrementY(sp, triangle_half);
//   for (u16 scan_y = pc.top_y;; scan_y += sp.scan_y_increment) {
//     assert(scan_y < kWindowHeight);
//     ::CalculateXScanlineBoundaries(sp, scan_y, pc, left_right_swapped);
//     ::CalculateInterpolationParametersForY(ip, scan_y, pc);
//     for (u16 scan_x = sp.scan_x_left; scan_x <= sp.scan_x_right; scan_x++) {
//       assert(scan_x < kWindowWidth);
//       ::CalculateInterpolationParametersForX(ip, sp, scan_x,
//                                              left_right_swapped);
//       if (ZBufferCheckAndReplace(ip.final_z, scan_y * kWindowWidth + scan_x,
//                                  render_buffer)) {
//         size_t index = scan_y * (pitch / kBytesPerPixel) + scan_x;
//         pixels[index] = argb_value;
//       }
//       if (scan_x == sp.scan_x_right)
//         break;
//     }
//     if (scan_y == pc.mid_y)
//       break;
//   }
// }

/***********************
 * Textured rasterizer *
 **********************/

// void TexturedRaster::RasterizeTriangles(
//     RasterizationContext& context) noexcept {
//   const Space& space = context.game_state.GetOutputSpace();
//   for (size_t t = 0; t < space.GetTriangleCount(); t++) {
//     const Triangle& triangle = *space.GetTriangles()[t];
//     // Triangle color
//     Direction light_direction = {1, 2, -3};
//     f32 brightness = light_direction.GetVector().normalized().dot(
//         triangle.GetNormal().normalized());
//     brightness = (brightness + 1.0f) / 2.0f;

//     PixelCoordinates pc;
//     OrderedVertexIndices vi;

//     ::SetSortedVertexIndices(vi, t, space);
//     ::SetPixelCoordinates(pc, vi, space);

//     // Scanlines for top section
//     RasterizeTriangleHalf(pc, vi, triangle, TriangleHalf::kUpper, brightness,
//                           context);
//     // Scanlines for bottom section
//     RasterizeTriangleHalf(pc, vi, triangle, TriangleHalf::kLower, brightness,
//                           context);
//   }
// }

// void TexturedRaster::RasterizeTriangleHalf(
//     PixelCoordinates& pc,
//     OrderedVertexIndices& vi,
//     const Triangle& triangle,
//     TriangleHalf triangle_half,
//     f32 brightness,
//     RasterizationContext& context) noexcept {
//   RenderBuffer& render_buffer = context.render_buffer;
//   const Texture& default_texture = context.default_texture;
//   Sample* const pixels = render_buffer.pixels;
//   const int pitch = render_buffer.pitch;
//   InterpolationParameters ip;
//   ScanlineParameters sp;
//   bool left_right_swapped;

//   const u16 texture_width = default_texture.GetWidth();
//   const u16 texture_height = default_texture.GetHeight();
//   const SDL_Surface* texture_surface = default_texture.GetSurface();
//   const int texture_pitch = texture_surface->pitch;

//   if (triangle_half == TriangleHalf::kLower)
//     ::SwapTopAndLow(pc, vi);

//   if (pc.low_y == pc.top_y || pc.top_y == pc.mid_y)  // #TODO: Best timing?
//     return;

//   const UVCoordinate& top_uv =
//       triangle.GetVertex(vi.top % kVerticesPerTriangle).GetUVCoordinate();
//   const UVCoordinate& mid_uv =
//       triangle.GetVertex(vi.mid % kVerticesPerTriangle).GetUVCoordinate();
//   const UVCoordinate& low_uv =
//       triangle.GetVertex(vi.low % kVerticesPerTriangle).GetUVCoordinate();

//   ::SetScanlineIncrementY(sp, triangle_half);
//   for (u16 scan_y = pc.top_y;; scan_y += sp.scan_y_increment) {
//     assert(scan_y < kWindowHeight);
//     ::CalculateXScanlineBoundaries(sp, scan_y, pc, left_right_swapped);
//     ::CalculateInterpolationParametersForY(ip, scan_y, pc);

//     Vector2 top_low_uv = (top_uv / ::TrueZ(pc.top_z)) * (1.0f - ip.top_low_t)
//     +
//                          (low_uv / ::TrueZ(pc.low_z)) * (ip.top_low_t);
//     top_low_uv *= ::TrueZ(ip.top_low_z);

//     Vector2 top_mid_uv = (top_uv / ::TrueZ(pc.top_z)) * (1.0f - ip.top_mid_t)
//     +
//                          (mid_uv / ::TrueZ(pc.mid_z)) * (ip.top_mid_t);
//     top_mid_uv *= ::TrueZ(ip.top_mid_z);

//     // Used by SIMD-intrinsics
//     alignas(64) std::array<u32, 2> pixel_buffer, target_pixel_offsets;
//     u8 counter = 0;

//     for (u16 scan_x = sp.scan_x_left; scan_x <= sp.scan_x_right; scan_x++) {
//       assert(scan_x < kWindowWidth);
//       ::CalculateInterpolationParametersForX(ip, sp, scan_x,
//                                              left_right_swapped);

//       Vector2 final_uv =
//           (top_low_uv / ::TrueZ(ip.top_low_z)) * (1 - ip.horizontal_t) +
//           (top_mid_uv / ::TrueZ(ip.top_mid_z)) * (ip.horizontal_t);
//       final_uv *= ::TrueZ(ip.final_z);

//       // #TODO: replace argument with incrementing
//       if (::ZBufferCheckAndReplace(ip.final_z, scan_y * kWindowWidth +
//       scan_x,
//                                    render_buffer)) {
//         u16 u = static_cast<u16>(final_uv[kU] * (texture_width - 1));
//         u16 v = static_cast<u16>((1 - final_uv[kV]) * (texture_height - 1));
//         assert(u < texture_width);
//         assert(v < texture_height);
//         u32* target_pixels = reinterpret_cast<u32*>(pixels);
//         u32* texture_pixels =
//         reinterpret_cast<u32*>(texture_surface->pixels); u32 target_offset =
//         scan_y * (pitch / kBytesPerPixel) + scan_x; u32 texture_offset = v *
//         (texture_pitch / kBytesPerPixel) + u;

//         // SIMD-intrinsics implementation
//         pixel_buffer[counter] = texture_pixels[texture_offset];
//         target_pixel_offsets[counter++] = target_offset;
//         if (counter == 2) {
//           __m128i u8_values =
//               _mm_loadl_epi64(reinterpret_cast<__m128i*>(pixel_buffer.data()));
//           __m256i packed_32 = _mm256_cvtepu8_epi32((u8_values));
//           __m256 f32_values = _mm256_cvtepi32_ps(packed_32);
//           __m256 brightness_vector = _mm256_set1_ps(brightness);
//           f32_values = _mm256_mul_ps(f32_values, brightness_vector);
//           packed_32 = _mm256_cvtps_epi32(f32_values);
//           __m256i mask = _mm256_setr_epi8(
//               0, 4, 8, 12, 16, 20, 24, 28, -1, -1, -1, -1, -1, -1, -1, -1,
//               -1, -1, -1, -1, -1, -1, -1, -1, 0, 4, 8, 12, 16, 20, 24, 28);
//           packed_32 = _mm256_shuffle_epi8(packed_32, mask);

//           mask = _mm256_setr_epi32(0, 6, -1, -1, -1, -1, -1, -1);
//           packed_32 = _mm256_permutevar8x32_epi32(packed_32, mask);

//           _mm_storel_epi64(reinterpret_cast<__m128i*>(pixel_buffer.data()),
//                            _mm256_castsi256_si128(packed_32));

//           for (size_t k = 0; k < 2; k++) {
//             target_pixels[target_pixel_offsets[k]] = pixel_buffer[k];
//           }
//           counter = 0;
//         }
//       }
//     }
//     if (scan_y == pc.mid_y)
//       break;
//   }
// }

void PureRasterizer::RasterizeGameState(
    RasterizationContext& context) noexcept {
  BackgroundFill::SetupPixels(context.render_buffer);
  ResetZBuffer::SetupZBuffer(context.render_buffer);
  RasterizeTriangles(&context);
}

void PureRasterizer::RasterizeTriangles(
    RasterizationContext* __restrict__ context) noexcept {
  const Space& space = context->game_state.GetOutputSpace();
  for (size_t t = 0; t < space.GetTriangleCount(); t++) {
    const auto triangle = space.GetTriangles()[t];
    // Triangle color
    Direction light_direction = {1, 2, -3};
    f32 brightness = light_direction.GetVector().normalized().dot(
        triangle->GetNormal().normalized());
    brightness = (brightness + 1.0f) / 2.0f;

    const Texture* const default_texture = &context->default_texture;
    Sample* const pixels = context->render_buffer.pixels;
    const int pitch = context->render_buffer.pitch;
    f32* const z_buffer = context->render_buffer.z_buffer;

    PixelCoordinates pc;
    OrderedVertexIndices vi;

    ::SetSortedVertexIndices(vi, t, space);
    ::SetPixelCoordinates(pc, vi, space);

    // Scanlines for top section
    RasterizeTriangleHalf(pc, vi, triangle, TriangleHalf::kUpper, brightness,
                          default_texture, pixels, pitch, z_buffer);
    // Scanlines for bottom section
    RasterizeTriangleHalf(pc, vi, triangle, TriangleHalf::kLower, brightness,
                          default_texture, pixels, pitch, z_buffer);
  }
}

void PureRasterizer::RasterizeTriangleHalf(
    PixelCoordinates& pc,
    OrderedVertexIndices& vi,
    const TriangleSharedPointer& triangle,
    TriangleHalf triangle_half,
    f32 brightness,
    const Texture* __restrict__ default_texture,
    Sample* const __restrict__ pixels,
    int pitch,
    f32* const __restrict__ z_buffer) noexcept {
  InterpolationParameters ip;
  ScanlineParameters sp;
  bool left_right_swapped;

  const u16 texture_width = default_texture->GetWidth();
  const u16 texture_height = default_texture->GetHeight();
  const SDL_Surface* texture_surface = default_texture->GetSurface();
  const int texture_pitch = texture_surface->pitch;

  if (triangle_half == TriangleHalf::kLower)
    ::SwapTopAndLow(pc, vi);

  if (pc.low_y == pc.top_y || pc.top_y == pc.mid_y)  // #TODO: Best timing?
    return;

  const UVCoordinate& top_uv =
      triangle->GetVertex(vi.top % kVerticesPerTriangle).GetUVCoordinate();
  const UVCoordinate& mid_uv =
      triangle->GetVertex(vi.mid % kVerticesPerTriangle).GetUVCoordinate();
  const UVCoordinate& low_uv =
      triangle->GetVertex(vi.low % kVerticesPerTriangle).GetUVCoordinate();

  ::SetScanlineIncrementY(sp, triangle_half);
  for (u16 scan_y = pc.top_y;; scan_y += sp.scan_y_increment) {
    assert(scan_y < kWindowHeight);
    ::CalculateXScanlineBoundaries(sp, scan_y, pc, left_right_swapped);
    ::CalculateInterpolationParametersForY(ip, scan_y, pc);

    Vector2 top_low_uv = (top_uv / ::TrueZ(pc.top_z)) * (1.0f - ip.top_low_t) +
                         (low_uv / ::TrueZ(pc.low_z)) * (ip.top_low_t);
    top_low_uv *= ::TrueZ(ip.top_low_z);

    Vector2 top_mid_uv = (top_uv / ::TrueZ(pc.top_z)) * (1.0f - ip.top_mid_t) +
                         (mid_uv / ::TrueZ(pc.mid_z)) * (ip.top_mid_t);
    top_mid_uv *= ::TrueZ(ip.top_mid_z);

    // Used by SIMD-intrinsics
    alignas(64) std::array<u32, 2> pixel_buffer, target_pixel_offsets;
    u8 counter = 0;

    for (u16 scan_x = sp.scan_x_left; scan_x <= sp.scan_x_right; scan_x++) {
      assert(scan_x < kWindowWidth);
      ::CalculateInterpolationParametersForX(ip, sp, scan_x,
                                             left_right_swapped);

      Vector2 final_uv =
          (top_low_uv / ::TrueZ(ip.top_low_z)) * (1 - ip.horizontal_t) +
          (top_mid_uv / ::TrueZ(ip.top_mid_z)) * (ip.horizontal_t);
      final_uv *= ::TrueZ(ip.final_z);

      // #TODO: replace argument with incrementing
      if (::ZBufferCheckAndReplace(ip.final_z, scan_y * kWindowWidth + scan_x,
                                   z_buffer)) {
        u16 u = static_cast<u16>(final_uv[kU] * (texture_width - 1));
        u16 v = static_cast<u16>((1 - final_uv[kV]) * (texture_height - 1));
        assert(u < texture_width);
        assert(v < texture_height);
        u32* target_pixels = reinterpret_cast<u32*>(pixels);
        u32* texture_pixels = reinterpret_cast<u32*>(texture_surface->pixels);
        u32 target_offset = scan_y * (pitch / kBytesPerPixel) + scan_x;
        u32 texture_offset = v * (texture_pitch / kBytesPerPixel) + u;

        // SIMD-intrinsics implementation
        pixel_buffer[counter] = texture_pixels[texture_offset];
        target_pixel_offsets[counter++] = target_offset;
        if (counter == 2) {
          __m128i u8_values =
              _mm_loadl_epi64(reinterpret_cast<__m128i*>(pixel_buffer.data()));
          __m256i packed_32 = _mm256_cvtepu8_epi32((u8_values));
          __m256 f32_values = _mm256_cvtepi32_ps(packed_32);
          __m256 brightness_vector = _mm256_set1_ps(brightness);
          f32_values = _mm256_mul_ps(f32_values, brightness_vector);
          packed_32 = _mm256_cvtps_epi32(f32_values);
          __m256i mask = _mm256_setr_epi8(
              0, 4, 8, 12, 16, 20, 24, 28, -1, -1, -1, -1, -1, -1, -1, -1, -1,
              -1, -1, -1, -1, -1, -1, -1, 0, 4, 8, 12, 16, 20, 24, 28);
          packed_32 = _mm256_shuffle_epi8(packed_32, mask);

          mask = _mm256_setr_epi32(0, 6, -1, -1, -1, -1, -1, -1);
          packed_32 = _mm256_permutevar8x32_epi32(packed_32, mask);

          _mm_storel_epi64(reinterpret_cast<__m128i*>(pixel_buffer.data()),
                           _mm256_castsi256_si128(packed_32));

          for (size_t k = 0; k < 2; k++) {
            target_pixels[target_pixel_offsets[k]] = pixel_buffer[k];
          }
          counter = 0;
        }
      }
    }
    if (scan_y == pc.mid_y)
      break;
  }
}
