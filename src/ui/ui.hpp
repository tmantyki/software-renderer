#ifndef UI_HPP
#define UI_HPP

#include <SDL2/SDL.h>
#include <cstdint>

#include "server/game_state.hpp"

class UserInterface {
 public:
  UserInterface(uint16_t width = 800, uint16_t height = 800);
  ~UserInterface();
  UserInterface(const UserInterface&) = delete;
  UserInterface& operator=(UserInterface&) = delete;
  bool InitializeSdlObjects();
  void DestroySdlObjects();
  uint16_t GetWidth() const noexcept;
  uint16_t GetHeight() const noexcept;
  SDL_Window* GetSdlWindow() noexcept;
  SDL_Renderer* GetSdlRenderer() noexcept;
  SDL_Texture* GetSdlTexture() noexcept;

 private:
  const uint16_t width_;
  const uint16_t height_;
  bool sdl_objects_initialized_;
  SDL_Window* sdl_window_;
  SDL_Renderer* sdl_renderer_;
  SDL_Texture* sdl_texture_;
  // std::vector<std::vector<float>> z_buffer_;
};

#endif