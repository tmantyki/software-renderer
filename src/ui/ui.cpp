#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cassert>

#include "geometry/common.hpp"
#include "ui.hpp"

namespace {
void SetColor(SDL_Renderer* renderer, const SDL_Color color) noexcept {
  SDL_SetRenderDrawColor(renderer, color.r, color.b, color.g, 0xff);
}
}  // namespace

UserInterface::UserInterface() : UserInterface(true) {}

UserInterface::UserInterface(bool initialize_sdl_objects)
    : sdl_objects_initialized_(false) {
  if (initialize_sdl_objects)
    InitializeSdlObjects();
}

UserInterface::~UserInterface() {
  DestroySdlObjects();
}

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
  sdl_renderer_ = SDL_CreateRenderer(sdl_window_, -1, SDL_RENDERER_SOFTWARE);
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

uint16_t UserInterface::GetWidth() const noexcept {
  return width_;
}

uint16_t UserInterface::GetHeight() const noexcept {
  return height_;
}

void UserInterface::StartFrameRasterization(uint8_t** pixels,
                                            int* pitch) noexcept {
  SDL_LockTexture(sdl_texture_, NULL, reinterpret_cast<void**>(pixels), pitch);
}

void UserInterface::EndFrameRasterization() const noexcept {
  SDL_RenderCopy(sdl_renderer_, sdl_texture_, NULL, NULL);
  SDL_UnlockTexture(sdl_texture_);
}

void UserInterface::ClearWithBackgroundColor() const noexcept {
  constexpr SDL_Color kBackgroundColor = {0x40, 0x40, 0x40, 0xff};
  constexpr SDL_Color kForegroundColor = {0xff, 0xff, 0xff, 0xff};
  ::SetColor(sdl_renderer_, kBackgroundColor);
  SDL_RenderClear(sdl_renderer_);
  ::SetColor(sdl_renderer_, kForegroundColor);
}

void UserInterface::RenderPresent() const noexcept {
  SDL_RenderPresent(sdl_renderer_);
}

void UserInterface::DrawLine(int x1, int y1, int x2, int y2) const noexcept {
  SDL_RenderDrawLine(sdl_renderer_, x1, y1, x2, y2);
}

BenchmarkInterface::BenchmarkInterface() noexcept
    : UserInterface(false), pitch_(kWindowWidth * kBytesPerPixel) {}

void BenchmarkInterface::StartFrameRasterization(uint8_t** pixels,
                                                 int* pitch) noexcept {
  *pixels = pixels_;
  *pitch = pitch_;
}

void BenchmarkInterface::EndFrameRasterization() const noexcept {}
void BenchmarkInterface::ClearWithBackgroundColor() const noexcept {}
void BenchmarkInterface::RenderPresent() const noexcept {}

void BenchmarkInterface::DrawLine(int x1,
                                  int y1,
                                  int x2,
                                  int y2) const noexcept {
  (void)x1;
  (void)y1;
  (void)x2;
  (void)y2;
}