#pragma once

#include <SDL2/SDL.h>
#include <cstdint>

#include "geometry/common.hpp"
#include "rasterizer/render_buffer.hpp"

class UserInterface {
 public:
  UserInterface() noexcept : UserInterface(true) {}
  UserInterface(bool initialize_sdl_objects) : sdl_objects_initialized_(false) {
    if (initialize_sdl_objects)
      InitializeSdlObjects();
  }
  ~UserInterface() { DestroySdlObjects(); }
  UserInterface(const UserInterface&) = delete;
  UserInterface& operator=(UserInterface&) = delete;
  uint16_t GetWidth() const noexcept { return width_; }
  uint16_t GetHeight() const noexcept { return height_; }
  int GetPitch() const noexcept { return pitch_; }
  // virtual void StartFrameRasterization(uint8_t** pixels, int* pitch) noexcept
  // {
  //   SDL_LockTexture(sdl_texture_, NULL, reinterpret_cast<void**>(pixels),
  //                   pitch);
  // }
  // virtual void EndFrameRasterization() const noexcept {
  //   SDL_RenderCopy(sdl_renderer_, sdl_texture_, NULL, NULL);
  //   SDL_UnlockTexture(sdl_texture_);
  // }
  // virtual void ClearWithBackgroundColor() const noexcept;
  void RenderPresent(const RenderBuffer& render_buffer) const noexcept {
    SDL_UpdateTexture(sdl_texture_, NULL,
                      static_cast<void*>(render_buffer.pixels),
                      render_buffer.pitch);
    SDL_RenderCopy(sdl_renderer_, sdl_texture_, NULL, NULL);
    SDL_RenderPresent(sdl_renderer_);
  }
  // virtual void DrawLine(int x1, int y1, int x2, int y2) const noexcept {
  //   SDL_RenderDrawLine(sdl_renderer_, x1, y1, x2, y2);
  // }

 protected:
  const uint16_t width_ = kWindowWidth;
  const uint16_t height_ = kWindowHeight;

 private:
  bool InitializeSdlObjects();
  void DestroySdlObjects();
  bool sdl_objects_initialized_;
  SDL_Window* sdl_window_;
  SDL_Renderer* sdl_renderer_;
  SDL_Texture* sdl_texture_;
  int pitch_;
};

/* class BenchmarkInterface : public UserInterface {
 public:
  BenchmarkInterface() noexcept
      : UserInterface(false), pitch_(kWindowWidth * kBytesPerPixel) {}
  virtual void StartFrameRasterization(uint8_t** pixels,
                                       int* pitch) noexcept override {
    *pixels = pixels_;
    *pitch = pitch_;
  }
  virtual void EndFrameRasterization() const noexcept override {}
  virtual void ClearWithBackgroundColor() const noexcept override {}
  virtual void RenderPresent() const noexcept override {}
  virtual void DrawLine(int x1,
                        int y1,
                        int x2,
                        int y2) const noexcept override {
    (void)x1;
    (void)y1;
    (void)x2;
    (void)y2;
  }

 private:
  uint8_t pixels_[kWindowWidth * kWindowHeight * kBytesPerPixel];
  int pitch_;
}; */
