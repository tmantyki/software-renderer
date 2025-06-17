#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_render.h>
#include <cassert>

#include "geometry/common.hpp"
#include "ui.hpp"

namespace {
// void SetColor(SDL_Renderer* renderer, const SDL_Color color) noexcept {
//   SDL_SetRenderDrawColor(renderer, color.r, color.b, color.g, 0xff);
// }
}  // namespace

// #TODO: use exception instead of bool return value
bool UserInterface::InitializeSdlObjects() {
  assert(!sdl_objects_initialized_);
  if (SDL_Init(SDL_INIT_VIDEO))
    return false;
  sdl_window_ = SDL_CreateWindow("Software Renderer", SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED, width_, height_, 0);
  if (!sdl_window_) {
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_Quit();
    return false;
  }
  sdl_renderer_ = SDL_CreateRenderer(sdl_window_, -1, SDL_RENDERER_ACCELERATED);
  if (!sdl_renderer_) {
    SDL_DestroyWindow(sdl_window_);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_Quit();
    return false;
  }
  sdl_texture_ =
      SDL_CreateTexture(sdl_renderer_, kDefaultPixelFormat,
                        SDL_TEXTUREACCESS_STREAMING, width_, height_);
  if (!sdl_texture_) {
    SDL_DestroyRenderer(sdl_renderer_);
    SDL_DestroyWindow(sdl_window_);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_Quit();
    return false;
  }
  int img_init_flags = IMG_INIT_PNG;
  if (!(IMG_Init(img_init_flags) & img_init_flags)) {
    SDL_DestroyTexture(sdl_texture_);
    SDL_DestroyRenderer(sdl_renderer_);
    SDL_DestroyWindow(sdl_window_);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_Quit();
    return false;
  }

  // Set pitch_
  void* pixels;
  SDL_LockTexture(sdl_texture_, nullptr, &pixels, &pitch_);
  SDL_UnlockTexture(sdl_texture_);

  sdl_objects_initialized_ = true;
  return true;
}

void UserInterface::DestroySdlObjects() {
  if (!sdl_objects_initialized_)
    return;
  IMG_Quit();
  SDL_DestroyTexture(sdl_texture_);
  SDL_DestroyRenderer(sdl_renderer_);
  SDL_DestroyWindow(sdl_window_);
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
  SDL_Quit();
  sdl_objects_initialized_ = false;
}

// void UserInterface::ClearWithBackgroundColor() const noexcept {
//   constexpr SDL_Color kBackgroundColor = {0x40, 0x40, 0x40, 0xff};
//   constexpr SDL_Color kForegroundColor = {0xff, 0xff, 0xff, 0xff};
//   ::SetColor(sdl_renderer_, kBackgroundColor);
//   SDL_RenderClear(sdl_renderer_);
//   ::SetColor(sdl_renderer_, kForegroundColor);
// }
