#include <SDL2/SDL.h>
#include <cassert>

#include "ui.hpp"

UserInterface::UserInterface(uint16_t width, uint16_t height)
    : width_(width), height_(height), sdl_objects_initialized_(false) {}

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
  sdl_renderer_ = SDL_CreateRenderer(
      sdl_window_, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
  if (!sdl_renderer_) {
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

SDL_Window* UserInterface::GetSdlWindow() noexcept {
  assert(sdl_objects_initialized_);
  return sdl_window_;
}

SDL_Renderer* UserInterface::GetSdlRenderer() noexcept {
  assert(sdl_objects_initialized_);
  return sdl_renderer_;
}
