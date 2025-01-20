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

void SwapTopAndLow(PixelCoordinates& pc, OrderedVertexIndices& vi) noexcept {
  uint16_t tmp_x = pc.low_x;
  uint16_t tmp_y = pc.low_y;
  float tmp_z = pc.low_z;
  float tmp_i = vi.low;
  pc.low_x = pc.top_x;
  pc.low_y = pc.top_y;
  pc.low_z = pc.top_z;
  vi.low = vi.top;
  pc.top_x = tmp_x;
  pc.top_y = tmp_y;
  pc.top_z = tmp_z;
  vi.top = tmp_i;
}

void CalculateXScanlineBoundaries(ScanlineParameters& sp,
                                  const PixelCoordinates pc) noexcept {
  sp.scan_x_left = (sp.scan_y * pc.low_x - pc.top_x * sp.scan_y -
                    pc.top_y * pc.low_x + pc.top_x * pc.low_y) /
                   (pc.low_y - pc.top_y);
  sp.scan_x_right = (sp.scan_y * pc.mid_x - pc.top_x * sp.scan_y -
                     pc.top_y * pc.mid_x + pc.top_x * pc.mid_y) /
                    (pc.mid_y - pc.top_y);
  sp.scan_x_increment = sp.scan_x_right < sp.scan_x_left ? -1 : 1;
}

void CalculateInterpolationParametersForY(InterpolationParameters& ip,
                                          const ScanlineParameters& sp,
                                          const PixelCoordinates& pc) noexcept {
  float numerator = static_cast<float>(sp.scan_y - pc.top_y);
  ip.top_low_t = numerator / (pc.low_y - pc.top_y);
  ip.top_mid_t = numerator / (pc.mid_y - pc.top_y);
  ip.top_low_z = pc.top_z * (1 - ip.top_low_t) + pc.low_z * ip.top_low_t;
  ip.top_mid_z = pc.top_z * (1 - ip.top_mid_t) + pc.mid_z * ip.top_mid_t;
}

void CalculateInterpolationParametersForX(
    InterpolationParameters& ip,
    const ScanlineParameters& sp) noexcept {
  ip.horizontal_t = static_cast<float>(sp.scan_x - sp.scan_x_left) /
                    (sp.scan_x_right - sp.scan_x_left);
  ip.final_z =
      ip.top_low_z * (1 - ip.horizontal_t) + ip.top_mid_z * ip.horizontal_t;
}

void SetScanlineIncrementY(ScanlineParameters& sp,
                           const TriangleHalf triangle_half) noexcept {
  sp.scan_y_increment = triangle_half == TriangleHalf::kLower ? -1 : 1;
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

ScanlineRasterizer::ScanlineRasterizer() noexcept {}

void ScanlineRasterizer::RasterizeGameState(
    const GameState& game_state,
    UserInterface& user_interface) noexcept {
  const Space& space = game_state.GetOutputSpace();
  SDL_Renderer* renderer = user_interface.GetSdlRenderer();
  SDL_Texture* texture = user_interface.GetSdlTexture();

  ResetZBuffer();
  SDL_LockTexture(texture, NULL, reinterpret_cast<void**>(&pixels_), &pitch_);
  ClearRenderer();

  for (size_t t = 0; t < space.GetTriangleCount(); t++) {
    TriangleSharedPointer triangle = space.GetTriangles()[t];
    // Triangle color
    Direction light_direction = {1, 2, 3};
    float brightness = light_direction.GetVector().normalized().dot(
        triangle->GetNormal().normalized());
    brightness = (brightness + 1) / 2;
    uint8_t color_value = brightness * 0xff;

    PixelCoordinates pc;
    OrderedVertexIndices vi;

    SetSortedVertexIndices(vi, t, space);
    SetPixelCoordinates(pc, vi, space);

    // Scanlines for top section
    RasterizeTriangleHalf(pc, vi, triangle, TriangleHalf::kUpper, color_value);
    // Scanlines for bottom section
    RasterizeTriangleHalf(pc, vi, triangle, TriangleHalf::kLower, color_value);
  }
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_UnlockTexture(texture);
}

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

void ScanlineRasterizer::SetSortedVertexIndices(
    OrderedVertexIndices& vertex_indices,
    const size_t triangle_index,
    const Space& space) const noexcept {
  size_t a_index = triangle_index * kVerticesPerTriangle;
  size_t b_index = triangle_index * kVerticesPerTriangle + 1;
  size_t c_index = triangle_index * kVerticesPerTriangle + 2;
  vertex_indices.top = ::GetBoundaryVertexIndexByDimension(
      a_index, b_index, c_index, kY, BoundaryType::kMin, space.GetVertices());
  vertex_indices.low = ::GetBoundaryVertexIndexByDimension(
      a_index, b_index, c_index, kY, BoundaryType::kMax, space.GetVertices());
  if ((a_index != vertex_indices.top) && (a_index != vertex_indices.low))
    vertex_indices.mid = a_index;
  else if ((b_index != vertex_indices.top) && (b_index != vertex_indices.low))
    vertex_indices.mid = b_index;
  else
    vertex_indices.mid = c_index;
}

void ScanlineRasterizer::SetPixelCoordinates(
    PixelCoordinates& pc,
    const OrderedVertexIndices& vertex_indices,
    const Space& space) const noexcept {
  pc.top_x = space.GetVertices()(kX, vertex_indices.top);
  pc.top_y = space.GetVertices()(kY, vertex_indices.top);
  pc.top_z = space.GetVertices()(kZ, vertex_indices.top);
  pc.mid_x = space.GetVertices()(kX, vertex_indices.mid);
  pc.mid_y = space.GetVertices()(kY, vertex_indices.mid);
  pc.mid_z = space.GetVertices()(kZ, vertex_indices.mid);
  pc.low_x = space.GetVertices()(kX, vertex_indices.low);
  pc.low_y = space.GetVertices()(kY, vertex_indices.low);
  pc.low_z = space.GetVertices()(kZ, vertex_indices.low);
}

void ScanlineRasterizer::RasterizeTriangleHalf(
    PixelCoordinates& pc,
    OrderedVertexIndices& vi,
    const TriangleSharedPointer& triangle,
    TriangleHalf triangle_half,
    uint8_t color_value) noexcept {
  (void)triangle;
  uint8_t* pixels = pixels_;
  int pitch = pitch_;
  InterpolationParameters ip;
  ScanlineParameters sp;

  if (triangle_half == TriangleHalf::kLower)
    ::SwapTopAndLow(pc, vi);

  if (pc.low_y == pc.top_y || pc.top_y == pc.mid_y)  // #TODO: Best timing?
    return;

  ::SetScanlineIncrementY(sp, triangle_half);
  for (sp.scan_y = pc.top_y;; sp.scan_y += sp.scan_y_increment) {
    assert(sp.scan_y < kWindowHeight);
    ::CalculateXScanlineBoundaries(sp, pc);
    ::CalculateInterpolationParametersForY(ip, sp, pc);
    for (sp.scan_x = sp.scan_x_left;; sp.scan_x += sp.scan_x_increment) {
      assert(sp.scan_x < kWindowWidth);
      ::CalculateInterpolationParametersForX(ip, sp);
      if (ZBufferCheckAndReplace(ip.final_z,
                                 sp.scan_y * kWindowWidth + sp.scan_x)) {
        WritePixel(color_value, sp, pixels, pitch);
      }
      if (sp.scan_x == sp.scan_x_right)
        break;
    }
    if (sp.scan_y == pc.mid_y)
      break;
  }
}

void ScanlineRasterizer::WritePixel(uint8_t color_value,
                                    const ScanlineParameters& sp,
                                    uint8_t* pixels,
                                    int pitch) noexcept {
  size_t index = sp.scan_y * pitch + sp.scan_x * kBytesPerPixel;
  pixels[index] = color_value;
  pixels[index + 1] = color_value;
  pixels[index + 2] = color_value;
  pixels[index + 3] = 0xff;
}

FlatRasterizer::FlatRasterizer() noexcept : FlatRasterizer({1, 1, 1}) {}

FlatRasterizer::FlatRasterizer(Direction light_direction) noexcept
    : light_direction_(light_direction) {}

TexturedRasterizer::TexturedRasterizer() noexcept
    : texture_("assets/images/nova.png") {}

void TexturedRasterizer::RasterizeTriangleHalf(
    PixelCoordinates& pc,
    OrderedVertexIndices& vi,
    const TriangleSharedPointer& triangle,
    TriangleHalf triangle_half,
    uint8_t color_value) noexcept {
  (void)color_value;
  uint8_t* pixels = pixels_;
  int pitch = pitch_;
  InterpolationParameters ip;
  ScanlineParameters sp;

  if (triangle_half == TriangleHalf::kLower)
    ::SwapTopAndLow(pc, vi);

  if (pc.low_y == pc.top_y || pc.top_y == pc.mid_y)  // #TODO: Best timing?
    return;

  UVCoordinate top_uv =
      triangle->GetVertex(vi.top % kVerticesPerTriangle).GetUVCoordinate();
  UVCoordinate mid_uv =
      triangle->GetVertex(vi.mid % kVerticesPerTriangle).GetUVCoordinate();
  UVCoordinate low_uv =
      triangle->GetVertex(vi.low % kVerticesPerTriangle).GetUVCoordinate();

  ::SetScanlineIncrementY(sp, triangle_half);
  for (sp.scan_y = pc.top_y;; sp.scan_y += sp.scan_y_increment) {
    assert(sp.scan_y < kWindowHeight);
    ::CalculateXScanlineBoundaries(sp, pc);
    ::CalculateInterpolationParametersForY(ip, sp, pc);

    Vector2 top_low_uv = top_uv * pc.top_z * (1 - ip.top_low_t) +
                         low_uv * pc.low_z * (ip.top_low_t);
    top_low_uv /= ip.top_low_z;

    Vector2 top_mid_uv = top_uv * pc.top_z * (1 - ip.top_mid_t) +
                         mid_uv * pc.mid_z * (ip.top_mid_t);
    top_mid_uv /= ip.top_mid_z;

    for (sp.scan_x = sp.scan_x_left;; sp.scan_x += sp.scan_x_increment) {
      assert(sp.scan_x < kWindowWidth);
      ::CalculateInterpolationParametersForX(ip, sp);

      Vector2 final_uv = top_low_uv * ip.top_low_z * (1 - ip.horizontal_t) +
                         top_mid_uv * ip.top_mid_z * (ip.horizontal_t);
      final_uv /= ip.final_z;

      if (ZBufferCheckAndReplace(ip.final_z,
                                 sp.scan_y * kWindowWidth + sp.scan_x)) {
        WritePixel(sp, pixels, pitch, final_uv);
      }
      if (sp.scan_x == sp.scan_x_right)
        break;
    }
    if (sp.scan_y == pc.mid_y)
      break;
  }
}

void TexturedRasterizer::WritePixel(const ScanlineParameters& sp,
                                    uint8_t* pixels,
                                    int pitch,
                                    const Vector2& uv) noexcept {
  size_t index = sp.scan_y * pitch + sp.scan_x * kBytesPerPixel;
  pixels[index] = 0x00;
  pixels[index + 1] = 0xff * uv[1];
  pixels[index + 2] = 0xff * uv[0];
  pixels[index + 3] = 0xff;
}