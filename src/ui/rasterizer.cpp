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

void SwapApexAndBasePixelCoordinates(PixelCoordinates& pc) noexcept {
  uint16_t tmp_x = pc.base_x;
  uint16_t tmp_y = pc.base_y;
  float tmp_z = pc.base_z;
  pc.base_x = pc.apex_x;
  pc.base_y = pc.apex_y;
  pc.base_z = pc.apex_z;
  pc.apex_x = tmp_x;
  pc.apex_y = tmp_y;
  pc.apex_z = tmp_z;
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
    : light_direction_(light_direction), z_buffer_{0} {}

void FlatRasterizer::RasterizeGameState(
    const GameState& game_state,
    UserInterface& user_interface) noexcept {
  const Space& space = game_state.GetOutputSpace();
  SDL_Renderer* renderer = user_interface.GetSdlRenderer();
  SDL_Texture* texture = user_interface.GetSdlTexture();

  for (uint16_t x = 0; x < 800; x++)
    for (uint16_t y = 0; y < 800; y++)
      z_buffer_[x][y] = 1;

  uint8_t* pixels;
  int pitch;
  constexpr int kBytesPerPixel = 4;
  SDL_LockTexture(texture, NULL, reinterpret_cast<void**>(&pixels), &pitch);

  for (uint16_t x = 0; x < 800; x++)
    for (uint16_t y = 0; y < 800; y++)
      for (uint16_t b = 0; b <= 3; b++)
        pixels[y * pitch + x * kBytesPerPixel + b] = 0x80;

  for (size_t t = 0; t < space.GetTriangleCount(); t++) {
    // Triangle color
    float brightness =
        light_direction_.dot(space.GetTriangles()[t]->GetNormal());
    brightness /= (light_direction_.GetVector().norm() *
                   space.GetTriangles()[t]->GetNormal().norm());
    brightness = (brightness + 1) / 2;
    uint8_t alpha = 0xff;
    uint8_t red = brightness * 0xff;
    uint8_t green = brightness * 0xff;
    uint8_t blue = brightness * 0xff;

    size_t a_index = t * kVerticesPerTriangle;
    size_t b_index = t * kVerticesPerTriangle + 1;
    size_t c_index = t * kVerticesPerTriangle + 2;

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

    uint16_t top_x = space.GetVertices()(kX, top_y_index);
    uint16_t top_y = space.GetVertices()(kY, top_y_index);
    float top_z = space.GetVertices()(kZ, top_y_index);
    uint16_t mid_x = space.GetVertices()(kX, mid_y_index);
    uint16_t mid_y = space.GetVertices()(kY, mid_y_index);
    float mid_z = space.GetVertices()(kZ, mid_y_index);
    uint16_t low_x = space.GetVertices()(kX, low_y_index);
    uint16_t low_y = space.GetVertices()(kY, low_y_index);
    float low_z = space.GetVertices()(kZ, low_y_index);

    // Scanlines for top section
    if (low_y != top_y && top_y != mid_y)
      for (uint16_t scan_y = top_y; scan_y <= mid_y; scan_y++) {
        float top_low_t = static_cast<float>(scan_y - top_y) / (low_y - top_y);
        float top_mid_t = static_cast<float>(scan_y - top_y) / (mid_y - top_y);
        uint16_t scan_x1 =
            (scan_y * low_x - top_x * scan_y - top_y * low_x + top_x * low_y) /
            (low_y - top_y);
        uint16_t scan_x2 =
            (scan_y * mid_x - top_x * scan_y - top_y * mid_x + top_x * mid_y) /
            (mid_y - top_y);

        uint16_t scan_x_left = scan_x1 < scan_x2 ? scan_x1 : scan_x2;
        uint16_t scan_x_right = scan_x1 < scan_x2 ? scan_x2 : scan_x1;
        for (uint16_t scan_x = scan_x_left; scan_x <= scan_x_right; scan_x++) {
          float horizontal_t;
          if (scan_x1 < scan_x2)
            horizontal_t = static_cast<float>(scan_x - scan_x_left) /
                           (scan_x_right - scan_x_left);
          else
            horizontal_t = 1 - (static_cast<float>(scan_x - scan_x_left) /
                                (scan_x_right - scan_x_left));
          float top_low_z = top_z * (1 - top_low_t) + low_z * top_low_t;
          float top_mid_z = top_z * (1 - top_mid_t) + mid_z * top_mid_t;
          float final_z =
              top_low_z * (1 - horizontal_t) + top_mid_z * horizontal_t;
          if (final_z < z_buffer_[scan_x][scan_y]) {  // #TODO:Check order !!
            if (scan_x >= 800 || scan_y >= 800)
              continue;
            assert(scan_x >= 0);
            assert(scan_x < 800);
            assert(scan_y >= 0);
            assert(scan_y < 800);
            z_buffer_[scan_x][scan_y] = final_z;
            pixels[scan_y * pitch + scan_x * kBytesPerPixel] = blue;
            pixels[scan_y * pitch + scan_x * kBytesPerPixel + 1] = green;
            pixels[scan_y * pitch + scan_x * kBytesPerPixel + 2] = red;
            pixels[scan_y * pitch + scan_x * kBytesPerPixel + 3] = alpha;
          }
        }
      }

    // Scanlines for bottom section
    if (low_y != top_y && low_y != mid_y)
      for (uint16_t scan_y = mid_y; scan_y <= low_y; scan_y++) {
        float top_low_t = static_cast<float>(scan_y - top_y) / (low_y - top_y);
        float mid_low_t = static_cast<float>(scan_y - mid_y) / (low_y - mid_y);
        uint16_t scan_x1 =
            (top_x * scan_y - top_x * low_y - scan_y * low_x + top_y * low_x) /
            (top_y - low_y);
        uint16_t scan_x2 =
            (-scan_y * low_x + scan_y * mid_x - low_y * mid_x + low_x * mid_y) /
            (-low_y + mid_y);

        uint16_t scan_x_left = scan_x1 < scan_x2 ? scan_x1 : scan_x2;
        uint16_t scan_x_right = scan_x1 < scan_x2 ? scan_x2 : scan_x1;
        for (uint16_t scan_x = scan_x_left; scan_x <= scan_x_right; scan_x++) {
          float horizontal_t;
          if (scan_x1 < scan_x2)
            horizontal_t = static_cast<float>(scan_x - scan_x_left) /
                           (scan_x_right - scan_x_left);
          else
            horizontal_t = 1 - (static_cast<float>(scan_x - scan_x_left) /
                                (scan_x_right - scan_x_left));
          float top_low_z = top_z * (1 - top_low_t) + low_z * top_low_t;
          float mid_low_z = mid_z * (1 - mid_low_t) + low_z * mid_low_t;
          float final_z =
              top_low_z * (1 - horizontal_t) + mid_low_z * horizontal_t;
          if (final_z < z_buffer_[scan_x][scan_y]) {  // #TODO:Check order !!
            if (scan_x >= 800 || scan_y >= 800)
              continue;
            assert(scan_x >= 0);
            assert(scan_x < 800);
            assert(scan_y >= 0);
            assert(scan_y < 800);
            z_buffer_[scan_x][scan_y] = final_z;
            pixels[scan_y * pitch + scan_x * kBytesPerPixel] = blue;
            pixels[scan_y * pitch + scan_x * kBytesPerPixel + 1] = green;
            pixels[scan_y * pitch + scan_x * kBytesPerPixel + 2] = red;
            pixels[scan_y * pitch + scan_x * kBytesPerPixel + 3] = alpha;
          }
        }
      }
  }
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_UnlockTexture(texture);

  // SDL_RenderPresent(renderer);
}

void ScanlineRasterizer::ResetZBuffer() noexcept {
  constexpr static uint16_t width = 800;  // #TODO: find better way to define
  constexpr static uint16_t height = 800;
  for (uint16_t x = 0; x < width; x++)
    for (uint16_t y = 0; y < height; y++)
      z_buffer_[x][y] = 1;
}

void ScanlineRasterizer::ClearRenderer(uint8_t* pixels, int pitch) noexcept {
  constexpr static uint16_t width = 800;  // #TODO: find better way to define
  constexpr static uint16_t height = 800;
  for (uint16_t x = 0; x < width; x++)
    for (uint16_t y = 0; y < height; y++)
      for (uint16_t b = 0; b <= kNumberOfPixelChannels; b++)
        pixels[y * pitch + x * kBytesPerPixel + b] = 0x80;
}

void ScanlineRasterizer::CalculateTrianglePixelCoordinates(
    PixelCoordinates& pixel_coordinates,
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

  pixel_coordinates.apex_x = space.GetVertices()(kX, top_y_index);
  pixel_coordinates.apex_y = space.GetVertices()(kY, top_y_index);
  pixel_coordinates.apex_z = space.GetVertices()(kZ, top_y_index);
  pixel_coordinates.mid_x = space.GetVertices()(kX, mid_y_index);
  pixel_coordinates.mid_y = space.GetVertices()(kY, mid_y_index);
  pixel_coordinates.mid_z = space.GetVertices()(kZ, mid_y_index);
  pixel_coordinates.base_x = space.GetVertices()(kX, low_y_index);
  pixel_coordinates.base_y = space.GetVertices()(kY, low_y_index);
  pixel_coordinates.base_z = space.GetVertices()(kZ, low_y_index);
}

void ScanlineRasterizer::RasterizeTriangleHalf(
    size_t triangle_index,
    PixelCoordinates& pc,
    TriangleHalf triangle_half) noexcept {
  if (triangle_half == TriangleHalf::kLower)
    ::SwapApexAndBasePixelCoordinates(pc);

  if (pc.base_y == pc.apex_y || pc.apex_y == pc.mid_y)
    return;

  uint8_t increment = triangle_half == TriangleHalf::kLower ? -1 : 1;

  for (uint16_t scan_y = pc.apex_y; scan_y != pc.mid_y; scan_y += increment) {
    float top_low_t =
        static_cast<float>(scan_y - pc.apex_y) / (pc.base_y - pc.apex_y);
    float top_mid_t =
        static_cast<float>(scan_y - pc.apex_y) / (pc.mid_y - pc.apex_y);
    uint16_t scan_x1 = (scan_y * pc.base_x - pc.apex_x * scan_y -
                        pc.apex_y * pc.base_x + pc.apex_x * pc.base_y) /
                       (pc.base_y - pc.apex_y);
    uint16_t scan_x2 = (scan_y * pc.mid_x - pc.apex_x * scan_y -
                        pc.apex_y * pc.mid_x + pc.apex_x * pc.mid_y) /
                       (pc.mid_y - pc.apex_y);

    uint16_t scan_x_left = scan_x1 < scan_x2 ? scan_x1 : scan_x2;
    uint16_t scan_x_right = scan_x1 < scan_x2 ? scan_x2 : scan_x1;
    for (uint16_t scan_x = scan_x_left; scan_x <= scan_x_right; scan_x++) {
      float horizontal_t;
      if (scan_x1 < scan_x2)
        horizontal_t = static_cast<float>(scan_x - scan_x_left) /
                       (scan_x_right - scan_x_left);
      else
        horizontal_t = 1 - (static_cast<float>(scan_x - scan_x_left) /
                            (scan_x_right - scan_x_left));
      float top_low_z = pc.apex_z * (1 - top_low_t) + pc.base_z * top_low_t;
      float top_mid_z = pc.apex_z * (1 - top_mid_t) + pc.mid_z * top_mid_t;
      float final_z = top_low_z * (1 - horizontal_t) + top_mid_z * horizontal_t;
      if (final_z < z_buffer_[scan_x][scan_y]) {  // #TODO:Check order !!
        if (scan_x >= 800 || scan_y >= 800)
          continue;
        assert(scan_x >= 0);
        assert(scan_x < 800);
        assert(scan_y >= 0);
        assert(scan_y < 800);
        z_buffer_[scan_x][scan_y] = final_z;
        pixels[scan_y * pitch + scan_x * kBytesPerPixel] = blue;
        pixels[scan_y * pitch + scan_x * kBytesPerPixel + 1] = green;
        pixels[scan_y * pitch + scan_x * kBytesPerPixel + 2] = red;
        pixels[scan_y * pitch + scan_x * kBytesPerPixel + 3] = alpha;
      }
    }
  }
}

void ScanlineRasterizer::RasterizeGameState(
    const GameState& game_state,
    UserInterface& user_interface) noexcept {
  const Space& space = game_state.GetOutputSpace();
  SDL_Renderer* renderer = user_interface.GetSdlRenderer();
  SDL_Texture* texture = user_interface.GetSdlTexture();
  RasterizerContext rasterizer_context = {space, renderer, texture};

  uint8_t* pixels;
  int pitch;
  ResetZBuffer();
  SDL_LockTexture(texture, NULL, reinterpret_cast<void**>(&pixels), &pitch);
  ClearRenderer(pixels, pitch);

  for (size_t t = 0; t < space.GetTriangleCount(); t++) {
    
    // Triangle color
    Direction light_direction = {1, 2, 3};
    float brightness = light_direction.GetVector().normalized().dot(
        space.GetTriangles()[t]->GetNormal().normalized());
    brightness = (brightness + 1) / 2;
    uint8_t alpha = 0xff;
    uint8_t red = brightness * 0xff;
    uint8_t green = brightness * 0xff;
    uint8_t blue = brightness * 0xff;

    PixelCoordinates pixel_coordinates;
    CalculateTrianglePixelCoordinates(pixel_coordinates, space, t);

    // Scanlines for top section
    if (low_y != top_y && top_y != mid_y)

      for (uint16_t scan_y = top_y; scan_y <= mid_y; scan_y++) {
        float top_low_t = static_cast<float>(scan_y - top_y) / (low_y - top_y);
        float top_mid_t = static_cast<float>(scan_y - top_y) / (mid_y - top_y);
        uint16_t scan_x1 =
            (scan_y * low_x - top_x * scan_y - top_y * low_x + top_x * low_y) /
            (low_y - top_y);
        uint16_t scan_x2 =
            (scan_y * mid_x - top_x * scan_y - top_y * mid_x + top_x * mid_y) /
            (mid_y - top_y);

        uint16_t scan_x_left = scan_x1 < scan_x2 ? scan_x1 : scan_x2;
        uint16_t scan_x_right = scan_x1 < scan_x2 ? scan_x2 : scan_x1;
        for (uint16_t scan_x = scan_x_left; scan_x <= scan_x_right; scan_x++) {
          float horizontal_t;
          if (scan_x1 < scan_x2)
            horizontal_t = static_cast<float>(scan_x - scan_x_left) /
                           (scan_x_right - scan_x_left);
          else
            horizontal_t = 1 - (static_cast<float>(scan_x - scan_x_left) /
                                (scan_x_right - scan_x_left));
          float top_low_z = top_z * (1 - top_low_t) + low_z * top_low_t;
          float top_mid_z = top_z * (1 - top_mid_t) + mid_z * top_mid_t;
          float final_z =
              top_low_z * (1 - horizontal_t) + top_mid_z * horizontal_t;
          if (final_z < z_buffer_[scan_x][scan_y]) {  // #TODO:Check order !!
            if (scan_x >= 800 || scan_y >= 800)
              continue;
            assert(scan_x >= 0);
            assert(scan_x < 800);
            assert(scan_y >= 0);
            assert(scan_y < 800);
            z_buffer_[scan_x][scan_y] = final_z;
            pixels[scan_y * pitch + scan_x * kBytesPerPixel] = blue;
            pixels[scan_y * pitch + scan_x * kBytesPerPixel + 1] = green;
            pixels[scan_y * pitch + scan_x * kBytesPerPixel + 2] = red;
            pixels[scan_y * pitch + scan_x * kBytesPerPixel + 3] = alpha;
          }
        }
      }

    // Scanlines for bottom section
    if (low_y != top_y && low_y != mid_y)
      for (uint16_t scan_y = mid_y; scan_y <= low_y; scan_y++) {
        float top_low_t = static_cast<float>(scan_y - top_y) / (low_y - top_y);
        float mid_low_t = static_cast<float>(scan_y - mid_y) / (low_y - mid_y);
        uint16_t scan_x1 =
            (top_x * scan_y - top_x * low_y - scan_y * low_x + top_y * low_x) /
            (top_y - low_y);
        uint16_t scan_x2 =
            (-scan_y * low_x + scan_y * mid_x - low_y * mid_x + low_x * mid_y) /
            (-low_y + mid_y);

        uint16_t scan_x_left = scan_x1 < scan_x2 ? scan_x1 : scan_x2;
        uint16_t scan_x_right = scan_x1 < scan_x2 ? scan_x2 : scan_x1;
        for (uint16_t scan_x = scan_x_left; scan_x <= scan_x_right; scan_x++) {
          float horizontal_t;
          if (scan_x1 < scan_x2)
            horizontal_t = static_cast<float>(scan_x - scan_x_left) /
                           (scan_x_right - scan_x_left);
          else
            horizontal_t = 1 - (static_cast<float>(scan_x - scan_x_left) /
                                (scan_x_right - scan_x_left));
          float top_low_z = top_z * (1 - top_low_t) + low_z * top_low_t;
          float mid_low_z = mid_z * (1 - mid_low_t) + low_z * mid_low_t;
          float final_z =
              top_low_z * (1 - horizontal_t) + mid_low_z * horizontal_t;
          if (final_z < z_buffer_[scan_x][scan_y]) {  // #TODO:Check order !!
            if (scan_x >= 800 || scan_y >= 800)
              continue;
            assert(scan_x >= 0);
            assert(scan_x < 800);
            assert(scan_y >= 0);
            assert(scan_y < 800);
            z_buffer_[scan_x][scan_y] = final_z;
            pixels[scan_y * pitch + scan_x * kBytesPerPixel] = blue;
            pixels[scan_y * pitch + scan_x * kBytesPerPixel + 1] = green;
            pixels[scan_y * pitch + scan_x * kBytesPerPixel + 2] = red;
            pixels[scan_y * pitch + scan_x * kBytesPerPixel + 3] = alpha;
          }
        }
      }
  }
  SDL_RenderCopy(rasterizer_context.renderer, rasterizer_context.texture, NULL,
                 NULL);
  SDL_UnlockTexture(rasterizer_context.texture);
}