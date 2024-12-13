#ifndef UI_HPP
#define UI_HPP

#include <SDL2/SDL.h>
#include <cstdint>

class UserInterface {
 public:
  UserInterface(uint16_t width = 800, uint16_t height = 800);
  ~UserInterface();
  UserInterface(const UserInterface&) = delete;
  UserInterface& operator=(UserInterface&) = delete;
  bool InitializeSdlObjects();
  void DestroySdlObjects();

 private:
  const uint16_t width_;
  const uint16_t height_;
  bool sdl_objects_initialized_;
  SDL_Window* sdl_window_;
  SDL_Renderer* sdl_renderer_;
};

#endif