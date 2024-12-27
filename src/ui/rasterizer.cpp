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
  SDL_Color kBackgroundColor = {0x0, 0x0, 0x0, 0xff};
  SDL_Color kForegroundColor = {0xff, 0xff, 0xff, 0xff};
  const Space& space = game_state.GetOutputSpace();
  SDL_Renderer* renderer = user_interface.GetSdlRenderer();
  ::SetColor(renderer, kBackgroundColor);
  SDL_RenderClear(renderer);
  ::SetColor(renderer, kForegroundColor);
  for (size_t t = 0; t < space.GetTriangleCount(); t++) {
    size_t a_index = t * kVerticesPerTriangle;
    size_t b_index = t * kVerticesPerTriangle + 1;
    size_t c_index = t * kVerticesPerTriangle + 2;

    size_t min_y_index = ::GetBoundaryVertexIndexByDimension(
        a_index, b_index, c_index, kY, BoundaryType::kMin, space.GetVertices());
    size_t max_y_index = ::GetBoundaryVertexIndexByDimension(
        a_index, b_index, c_index, kY, BoundaryType::kMax, space.GetVertices());
    size_t mid_y_index;
    if ((a_index != min_y_index) && (a_index != max_y_index))
      mid_y_index = a_index;
    else if ((b_index != min_y_index) && (b_index != max_y_index))
      mid_y_index = b_index;
    else
      mid_y_index = c_index;

    uint16_t min_x = space.GetVertices()(kX, min_y_index);
    uint16_t min_y = space.GetVertices()(kY, min_y_index);
    uint16_t mid_x = space.GetVertices()(kX, mid_y_index);
    uint16_t mid_y = space.GetVertices()(kY, mid_y_index);
    uint16_t max_x = space.GetVertices()(kX, max_y_index);
    uint16_t max_y = space.GetVertices()(kY, max_y_index);

    // Scanlines for top section
    for (uint16_t scan_y = min_y; scan_y < mid_y; scan_y++) {
      float max_t = static_cast<float>(scan_y - min_y) / (max_y - min_y);
      float mid_t = static_cast<float>(scan_y - min_y) / (mid_y - min_y);
      uint16_t scan_x1 = min_x * (1 - max_t) + max_x * max_t;
      uint16_t scan_x2 = min_x * (1 - mid_t) + mid_x * mid_t;
      SDL_RenderDrawLine(renderer, scan_x1, scan_y, scan_x2, scan_y);
    }

    // Scanlines for bottom section
    for (uint16_t scan_y = mid_y; scan_y < max_y; scan_y++) {
      float max_t = static_cast<float>(scan_y - min_y) / (max_y - min_y);
      float mid_t = static_cast<float>(scan_y - mid_y) / (max_y - mid_y);
      uint16_t scan_x1 = min_x * (1 - max_t) + max_x * max_t;
      uint16_t scan_x2 = mid_x * (1 - mid_t) + max_x * mid_t;
      SDL_RenderDrawLine(renderer, scan_x1, scan_y, scan_x2, scan_y);
    }
  }
  SDL_RenderPresent(renderer);
}