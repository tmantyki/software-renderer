#include <SDL2/SDL.h>

#include "ui/rasterizer.hpp"

namespace {
void SetColor(SDL_Renderer* renderer, const SDL_Color color) noexcept {
  SDL_SetRenderDrawColor(renderer, color.r, color.b, color.g, 0xff);
}
}  // namespace

void WireframeRasterizer::RasterizeGameState(
    const GameState& game_state,
    UserInterface& user_interface) noexcept {
  SDL_Color kBackgroundColor = {0x4, 0x4, 0x4, 0xff};
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