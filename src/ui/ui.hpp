#ifndef UI_HPP
#define UI_HPP

#include <SDL2/SDL.h>
#include <cstdint>

#include "server/game_state.hpp"

class UserInterface {
 public:
  UserInterface();
  UserInterface(bool initialize_sdl_objects);
  ~UserInterface();
  UserInterface(const UserInterface&) = delete;
  UserInterface& operator=(UserInterface&) = delete;
  uint16_t GetWidth() const noexcept;
  uint16_t GetHeight() const noexcept;
  virtual void StartFrameRasterization(uint8_t** pixels, int* pitch) noexcept;
  virtual void EndFrameRasterization() const noexcept;
  virtual void ClearWithBackgroundColor() const noexcept;
  virtual void RenderPresent() const noexcept;
  virtual void DrawLine(int x1, int y1, int x2, int y2) const noexcept;

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
};

class BenchmarkInterface : public UserInterface {
 public:
  BenchmarkInterface() noexcept;
  virtual void StartFrameRasterization(uint8_t** pixels,
                                       int* pitch) noexcept override;
  virtual void EndFrameRasterization() const noexcept override;
  virtual void ClearWithBackgroundColor() const noexcept override;
  virtual void RenderPresent() const noexcept override;
  virtual void DrawLine(int x1, int y1, int x2, int y2) const noexcept override;

 private:
  uint8_t pixels_[kWindowWidth * kWindowHeight * kBytesPerPixel];
  int pitch_;
};

#endif