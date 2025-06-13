#include <SDL2/SDL.h>

#include <algorithm>
#include <cstdint>
#include "geometry/common.hpp"
#include "ui/rasterizer.hpp"

#include <emmintrin.h>
#include <immintrin.h>

namespace {
size_t GetBoundaryVertexIndexByDimension(size_t a_index,
                                         size_t b_index,
                                         size_t c_index,
                                         enum Axis axis,
                                         BoundaryType boundary_type,
                                         const VertexMatrix& vertices) {
  float a = vertices(axis, a_index);
  float b = vertices(axis, b_index);
  float c = vertices(axis, c_index);
  float boundary_value;
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

void SwapTopAndLow(PixelCoordinates& pc, OrderedVertexIndices& vi) noexcept {
  std::swap(pc.low_x, pc.top_x);
  std::swap(pc.low_y, pc.top_y);
  std::swap(pc.low_z, pc.top_z);
  std::swap(vi.low, vi.top);
}

void CalculateXScanlineBoundaries(ScanlineParameters& sp,
                                  uint16_t scan_y,
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

void CalculateInterpolationParametersForY(InterpolationParameters& ip,
                                          const uint16_t scan_y,
                                          const PixelCoordinates& pc) noexcept {
  float numerator = static_cast<float>(scan_y - pc.top_y);
  ip.top_low_t = numerator / (pc.low_y - pc.top_y);
  ip.top_mid_t = numerator / (pc.mid_y - pc.top_y);
  ip.top_low_z = pc.top_z * (1.0f - ip.top_low_t) + pc.low_z * ip.top_low_t;
  ip.top_mid_z = pc.top_z * (1.0f - ip.top_mid_t) + pc.mid_z * ip.top_mid_t;
}

void CalculateInterpolationParametersForX(
    InterpolationParameters& ip,
    const ScanlineParameters& sp,
    const uint16_t scan_x,
    const bool left_right_swapped) noexcept {
  ip.horizontal_t = static_cast<float>(scan_x - sp.scan_x_left) /
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
float TrueZ(float reciprocal_z) noexcept {
  constexpr float A = 2.0f * kNearPlaneDistance * kFarPlaneDistance /
                      (kFarPlaneDistance - kNearPlaneDistance);
  constexpr float B = (kFarPlaneDistance + kNearPlaneDistance) /
                      (kFarPlaneDistance - kNearPlaneDistance);
  return A / (reciprocal_z - B);
}
}  // namespace

void WireframeRasterizer::RasterizeGameState(
    const GameState& game_state,
    UserInterface& user_interface) noexcept {
  const Space& space = game_state.GetOutputSpace();
  user_interface.ClearWithBackgroundColor();
  const VertexMatrix& vertices = space.GetVertices();
  for (size_t t = 0; t < space.GetTriangleCount(); t++) {
    for (size_t a : {0, 1, 2}) {
      size_t b = (a + 1) % 3;
      uint16_t a_x = vertices(kX, t * kVerticesPerTriangle + a);
      uint16_t a_y = vertices(kY, t * kVerticesPerTriangle + a);
      uint16_t b_x = vertices(kX, t * kVerticesPerTriangle + b);
      uint16_t b_y = vertices(kY, t * kVerticesPerTriangle + b);
      user_interface.DrawLine(a_x, a_y, b_x, b_y);
    }
  }
}

void ScanlineRasterizer::RasterizeGameState(
    const GameState& game_state,
    UserInterface& user_interface) noexcept {
  const Space& space = game_state.GetOutputSpace();

  ResetZBuffer();
  user_interface.StartFrameRasterization(&pixels_, &pitch_);
  ClearRenderer();

  for (size_t t = 0; t < space.GetTriangleCount(); t++) {
    TriangleSharedPointer triangle = space.GetTriangles()[t];
    // Triangle color
    Direction light_direction = {1, 2, -3};
    float brightness = light_direction.GetVector().normalized().dot(
        triangle->GetNormal().normalized());
    brightness = (brightness + 1.0f) / 2.0f;

    PixelCoordinates pc;
    OrderedVertexIndices vi;

    SetSortedVertexIndices(vi, t, space);
    SetPixelCoordinates(pc, vi, space);

    // Scanlines for top section
    RasterizeTriangleHalf(pc, vi, triangle, TriangleHalf::kUpper, brightness);
    // Scanlines for bottom section
    RasterizeTriangleHalf(pc, vi, triangle, TriangleHalf::kLower, brightness);
  }
  user_interface.EndFrameRasterization();
}

void ScanlineRasterizer::ClearRenderer() noexcept {
  uint8_t* pixels = pixels_;
  int pitch = pitch_;
  uint32_t pixel_value = 0xff008080;
  for (uint16_t y = 0; y < kWindowHeight; y++)
    for (uint16_t x = 0; x < kWindowWidth * kBytesPerPixel;
         x += kBytesPerPixel) {
      *reinterpret_cast<uint32_t*>(pixels + y * pitch + x) = pixel_value;
    }
}

// #TODO: refactor with simpler sorting logic
void ScanlineRasterizer::SetSortedVertexIndices(
    OrderedVertexIndices& vertex_indices,
    const size_t triangle_index,
    const Space& space) const noexcept {
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
void ScanlineRasterizer::SetPixelCoordinates(
    PixelCoordinates& pc,
    const OrderedVertexIndices& vertex_indices,
    const Space& space) const noexcept {
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

void ScanlineRasterizer::RasterizeTriangleHalf(
    PixelCoordinates& pc,
    OrderedVertexIndices& vi,
    const TriangleSharedPointer& triangle,
    TriangleHalf triangle_half,
    float brightness) noexcept {
  (void)pc;
  (void)vi;
  (void)triangle;
  (void)triangle_half;
  (void)brightness;
  // uint8_t* pixels = pixels_;
  // int pitch = pitch_;
  // InterpolationParameters ip;
  // ScanlineParameters sp;

  // if (triangle_half == TriangleHalf::kLower)
  //   ::SwapTopAndLow(pc, vi);

  // if (pc.low_y == pc.top_y || pc.top_y == pc.mid_y)  // #TODO: Best timing?
  //   return;

  // ::SetScanlineIncrementY(sp, triangle_half);
  // for (sp.scan_y = pc.top_y;; sp.scan_y += sp.scan_y_increment) {
  //   assert(sp.scan_y < kWindowHeight);
  //   ::CalculateXScanlineBoundaries(sp, pc);
  //   ::CalculateInterpolationParametersForY(ip, sp, pc);
  //   for (sp.scan_x = sp.scan_x_left;; sp.scan_x += sp.scan_x_increment) {
  //     assert(sp.scan_x < kWindowWidth);
  //     ::CalculateInterpolationParametersForX(ip, sp);
  //     if (ZBufferCheckAndReplace(ip.final_z,
  //                                sp.scan_y * kWindowWidth + sp.scan_x)) {
  //       WritePixel(color_value, sp, pixels, pitch);
  //     }
  //     if (sp.scan_x == sp.scan_x_right)
  //       break;
  //   }
  //   if (sp.scan_y == pc.mid_y)
  //     break;
  // }
}

// void ScanlineRasterizer::WritePixel(uint8_t color_value,
//                                     const ScanlineParameters& sp,
//                                     uint8_t* pixels,
//                                     int pitch) noexcept {
//   size_t index = sp.scan_y * pitch + sp.scan_x * kBytesPerPixel;
//   pixels[index] = color_value;
//   pixels[index + 1] = color_value;
//   pixels[index + 2] = color_value;
//   pixels[index + 3] = 0xff;
// }

void TexturedRasterizer::RasterizeTriangleHalf(
    PixelCoordinates& pc,
    OrderedVertexIndices& vi,
    const TriangleSharedPointer& triangle,
    TriangleHalf triangle_half,
    float brightness) noexcept {
  uint8_t* pixels = pixels_;
  int pitch = pitch_;
  InterpolationParameters ip;
  ScanlineParameters sp;
  bool left_right_swapped;

  const uint16_t texture_width = texture_.GetWidth();
  const uint16_t texture_height = texture_.GetHeight();
  const SDL_Surface* texture_surface = texture_.GetSurface();
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
  for (uint16_t scan_y = pc.top_y;; scan_y += sp.scan_y_increment) {
    assert(scan_y < kWindowHeight);
    ::CalculateXScanlineBoundaries(sp, scan_y, pc, left_right_swapped);
    ::CalculateInterpolationParametersForY(ip, scan_y, pc);

    Vector2 top_low_uv = (top_uv / ::TrueZ(pc.top_z)) * (1.0f - ip.top_low_t) +
                         (low_uv / ::TrueZ(pc.low_z)) * (ip.top_low_t);
    top_low_uv *= ::TrueZ(ip.top_low_z);

    Vector2 top_mid_uv = (top_uv / ::TrueZ(pc.top_z)) * (1.0f - ip.top_mid_t) +
                         (mid_uv / ::TrueZ(pc.mid_z)) * (ip.top_mid_t);
    top_mid_uv *= ::TrueZ(ip.top_mid_z);

    alignas(64) std::array<uint32_t, 2> pixel_buffer, target_pixel_offsets;
    uint8_t counter = 0;

    for (uint16_t scan_x = sp.scan_x_left; scan_x <= sp.scan_x_right;
         scan_x++) {
      assert(scan_x < kWindowWidth);
      ::CalculateInterpolationParametersForX(ip, sp, scan_x,
                                             left_right_swapped);

      Vector2 final_uv =
          (top_low_uv / ::TrueZ(ip.top_low_z)) * (1 - ip.horizontal_t) +
          (top_mid_uv / ::TrueZ(ip.top_mid_z)) * (ip.horizontal_t);
      final_uv *= ::TrueZ(ip.final_z);

      // #TODO: replace argument with incrementing
      if (ZBufferCheckAndReplace(ip.final_z, scan_y * kWindowWidth + scan_x)) {
        uint16_t u = static_cast<uint16_t>(final_uv[kU] * (texture_width - 1));
        uint16_t v =
            static_cast<uint16_t>((1 - final_uv[kV]) * (texture_height - 1));
        assert(u < texture_width);
        assert(v < texture_height);
        uint32_t* target_pixels = reinterpret_cast<uint32_t*>(pixels);
        uint32_t* texture_pixels =
            reinterpret_cast<uint32_t*>(texture_surface->pixels);
        uint32_t target_offset = scan_y * (pitch / kBytesPerPixel) + scan_x;
        uint32_t texture_offset = v * (texture_pitch / kBytesPerPixel) + u;

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
          __m256i mask = _mm256_setr_epi8(0, 4, 8, 16, 20, 24, 28, 32, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0);
          packed_32 = _mm256_shuffle_epi8(packed_32, mask);
          _mm_storel_epi64(reinterpret_cast<__m128i*>(pixel_buffer.data()),
                           u8_values);

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
