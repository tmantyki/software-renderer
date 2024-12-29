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
      int16_t a_x = space.GetVertices()(0, t * kVerticesPerTriangle + a);
      int16_t a_y = space.GetVertices()(1, t * kVerticesPerTriangle + a);
      int16_t b_x = space.GetVertices()(0, t * kVerticesPerTriangle + b);
      int16_t b_y = space.GetVertices()(1, t * kVerticesPerTriangle + b);
      SDL_RenderDrawLine(renderer, a_x, a_y, b_x, b_y);
    }
  }
  SDL_RenderPresent(renderer);
}

FlatRasterizer::FlatRasterizer() noexcept : FlatRasterizer({0, -1, 0}) {}

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
    for (uint16_t scan_y = top_y; scan_y < mid_y; scan_y++) {
      float top_low_t = static_cast<float>(scan_y - top_y) / (low_y - top_y);
      float top_mid_t = static_cast<float>(scan_y - top_y) / (mid_y - top_y);
      uint16_t scan_x1 = top_x * (1 - top_low_t) + low_x * top_low_t;
      uint16_t scan_x2 = top_x * (1 - top_mid_t) + mid_x * top_mid_t;

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
        float final_z = top_low_z * (1 - horizontal_t) + mid_z * horizontal_t;
        if (final_z < z_buffer_[scan_x][scan_y]) {  // #TODO:Check order !!
          if (scan_x == 800)
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
    for (uint16_t scan_y = mid_y; scan_y < low_y; scan_y++) {
      float top_low_t = static_cast<float>(scan_y - top_y) / (low_y - top_y);
      float mid_low_t = static_cast<float>(scan_y - mid_y) / (low_y - mid_y);
      uint16_t scan_x1 = top_x * (1 - top_low_t) + low_x * top_low_t;
      uint16_t scan_x2 = mid_x * (1 - mid_low_t) + low_x * mid_low_t;

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
        float final_z = top_low_z * (1 - horizontal_t) + mid_z * horizontal_t;
        if (final_z < z_buffer_[scan_x][scan_y]) {  // #TODO:Check order !!
          if (scan_x == 800)
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