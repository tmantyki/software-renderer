#include <SDL2/SDL.h>

#include "ui/rasterizer.hpp"

namespace {
void SetColor(SDL_Renderer* renderer, const SDL_Color color) noexcept {
  SDL_SetRenderDrawColor(renderer, color.r, color.b, color.g, 0xff);
}

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

void SwapTopAndLowPixelCoordinates(PixelCoordinates& pc) noexcept {
  uint16_t tmp_x = pc.low_x;
  uint16_t tmp_y = pc.low_y;
  float tmp_z = pc.low_z;
  pc.low_x = pc.top_x;
  pc.low_y = pc.top_y;
  pc.low_z = pc.top_z;
  pc.top_x = tmp_x;
  pc.top_y = tmp_y;
  pc.top_z = tmp_z;
}

}  // namespace

void WireframeRasterizer::RasterizeGameState(
    const GameState& game_state,
    UserInterface& user_interface) noexcept {
  SDL_Color kBackgroundColor = {0x40, 0x40, 0x40, 0xff};
  SDL_Color kForegroundColor = {0xff, 0xff, 0xff, 0xff};
  const Space& space = game_state.GetOutputSpace();
  SDL_Renderer* renderer = user_interface.GetSdlRenderer();
  ::SetColor(renderer, kBackgroundColor);
  SDL_RenderClear(renderer);
  ::SetColor(renderer, kForegroundColor);
  for (size_t t = 0; t < space.GetTriangleCount(); t++) {
    for (size_t a : {0, 1, 2}) {
      size_t b = (a + 1) % 3;
      uint16_t a_x = space.GetVertices()(kX, t * kVerticesPerTriangle + a);
      uint16_t a_y = space.GetVertices()(kY, t * kVerticesPerTriangle + a);
      uint16_t b_x = space.GetVertices()(kX, t * kVerticesPerTriangle + b);
      uint16_t b_y = space.GetVertices()(kY, t * kVerticesPerTriangle + b);
      SDL_RenderDrawLine(renderer, a_x, a_y, b_x, b_y);
    }
  }
  SDL_RenderPresent(renderer);
}

FlatRasterizer::FlatRasterizer() noexcept : FlatRasterizer({1, 1, 1}) {}

FlatRasterizer::FlatRasterizer(Direction light_direction) noexcept
    : light_direction_(light_direction) {}

ScanlineRasterizer::ScanlineRasterizer() noexcept {}

void ScanlineRasterizer::ResetZBuffer() noexcept {
  for (uint32_t i = 0; i < kWindowWidth * kWindowHeight; i++)
    z_buffer_[i] = 1;
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

bool ScanlineRasterizer::ZBufferCheckAndReplace(
    float new_value,
    uint32_t z_buffer_index) noexcept {
  if (new_value - 0.001 < z_buffer_[z_buffer_index]) {
    z_buffer_[z_buffer_index] = new_value;
    return true;
  } else
    return false;
}

void ScanlineRasterizer::CalculateTrianglePixelCoordinates(
    PixelCoordinates& pc,
    const Space& space,
    size_t triangle_index) const noexcept {
  size_t a_index = triangle_index * kVerticesPerTriangle;
  size_t b_index = triangle_index * kVerticesPerTriangle + 1;
  size_t c_index = triangle_index * kVerticesPerTriangle + 2;

  size_t top_y_index = ::GetBoundaryVertexIndexByDimension(
      a_index, b_index, c_index, kY, BoundaryType::kMin, space.GetVertices());
  size_t low_y_index = ::GetBoundaryVertexIndexByDimension(
      a_index, b_index, c_index, kY, BoundaryType::kMax, space.GetVertices());
  size_t mid_y_index;
  if ((a_index != top_y_index) && (a_index != low_y_index))
    mid_y_index = a_index;
  else if ((b_index != top_y_index) && (b_index != low_y_index))
    mid_y_index = b_index;
  else
    mid_y_index = c_index;

  pc.top_x = space.GetVertices()(kX, top_y_index);
  pc.top_y = space.GetVertices()(kY, top_y_index);
  pc.top_z = space.GetVertices()(kZ, top_y_index);
  pc.mid_x = space.GetVertices()(kX, mid_y_index);
  pc.mid_y = space.GetVertices()(kY, mid_y_index);
  pc.mid_z = space.GetVertices()(kZ, mid_y_index);
  pc.low_x = space.GetVertices()(kX, low_y_index);
  pc.low_y = space.GetVertices()(kY, low_y_index);
  pc.low_z = space.GetVertices()(kZ, low_y_index);
}

void ScanlineRasterizer::CalculateXScanlineBoundaries(
    const uint16_t scan_y,
    const PixelCoordinates pc,
    uint16_t& scan_x_left,
    uint16_t& scan_x_right) const noexcept {
  scan_x_left = (scan_y * pc.low_x - pc.top_x * scan_y - pc.top_y * pc.low_x +
                 pc.top_x * pc.low_y) /
                (pc.low_y - pc.top_y);
  scan_x_right = (scan_y * pc.mid_x - pc.top_x * scan_y - pc.top_y * pc.mid_x +
                  pc.top_x * pc.mid_y) /
                 (pc.mid_y - pc.top_y);
}

void ScanlineRasterizer::CalculateInterpolationParametersForY(
    const uint16_t scan_y,
    InterpolationParameters& ip,
    const PixelCoordinates& pc) const noexcept {
  float numerator = static_cast<float>(scan_y - pc.top_y);
  ip.top_low_t = numerator / (pc.low_y - pc.top_y);
  ip.top_mid_t = numerator / (pc.mid_y - pc.top_y);
  ip.top_low_z = pc.top_z * (1 - ip.top_low_t) + pc.low_z * ip.top_low_t;
  ip.top_mid_z = pc.top_z * (1 - ip.top_mid_t) + pc.mid_z * ip.top_mid_t;
}

void ScanlineRasterizer::RasterizeTriangleHalf(size_t triangle_index,
                                               PixelCoordinates& pc,
                                               TriangleHalf triangle_half,
                                               float brightness) noexcept {
  (void)triangle_index;
  uint8_t* pixels = pixels_;
  int pitch = pitch_;

  if (triangle_half == TriangleHalf::kLower)
    ::SwapTopAndLowPixelCoordinates(pc);

  if (pc.low_y == pc.top_y || pc.top_y == pc.mid_y)
    return;

  int8_t scan_y_increment = triangle_half == TriangleHalf::kLower ? -1 : 1;
  for (uint16_t scan_y = pc.top_y;;) {
    assert(scan_y < kWindowHeight);
    InterpolationParameters ip;
    CalculateInterpolationParametersForY(scan_y, ip, pc);
    uint16_t scan_x_left, scan_x_right;
    CalculateXScanlineBoundaries(scan_y, pc, scan_x_left, scan_x_right);
    int8_t scan_x_increment = scan_x_right < scan_x_left ? -1 : 1;
    for (uint16_t scan_x = scan_x_left;;) {
      assert(scan_x < kWindowWidth);
      float horizontal_t = static_cast<float>(scan_x - scan_x_left) /
                           (scan_x_right - scan_x_left);
      float final_z =
          ip.top_low_z * (1 - horizontal_t) + ip.top_mid_z * horizontal_t;
      if (ZBufferCheckAndReplace(final_z, scan_y * kWindowWidth + scan_x)) {
        pixels[scan_y * pitch + scan_x * kBytesPerPixel] = brightness * 0xff;
        pixels[scan_y * pitch + scan_x * kBytesPerPixel + 1] =
            brightness * 0xff;
        pixels[scan_y * pitch + scan_x * kBytesPerPixel + 2] =
            brightness * 0xff;
        pixels[scan_y * pitch + scan_x * kBytesPerPixel + 3] = 0xff;
      }
      if (scan_x == scan_x_right)
        break;
      scan_x += scan_x_increment;
    }
    if (scan_y == pc.mid_y)
      break;
    scan_y += scan_y_increment;
  }
}

void ScanlineRasterizer::RasterizeGameState(
    const GameState& game_state,
    UserInterface& user_interface) noexcept {
  const Space& space = game_state.GetOutputSpace();
  SDL_Renderer* renderer = user_interface.GetSdlRenderer();
  SDL_Texture* texture = user_interface.GetSdlTexture();
  RasterizerContext rasterizer_context = {space, renderer, texture};

  ResetZBuffer();
  SDL_LockTexture(texture, NULL, reinterpret_cast<void**>(&pixels_), &pitch_);
  ClearRenderer();

  for (size_t t = 0; t < space.GetTriangleCount(); t++) {
    // Triangle color
    Direction light_direction = {1, 2, 3};
    float brightness = light_direction.GetVector().normalized().dot(
        space.GetTriangles()[t]->GetNormal().normalized());
    brightness = (brightness + 1) / 2;

    PixelCoordinates pixel_coordinates;
    CalculateTrianglePixelCoordinates(pixel_coordinates, space, t);

    // Scanlines for top section
    RasterizeTriangleHalf(t, pixel_coordinates, TriangleHalf::kUpper,
                          brightness);
    // Scanlines for bottom section
    RasterizeTriangleHalf(t, pixel_coordinates, TriangleHalf::kLower,
                          brightness);
  }
  SDL_RenderCopy(rasterizer_context.renderer, rasterizer_context.texture, NULL,
                 NULL);
  SDL_UnlockTexture(rasterizer_context.texture);
}