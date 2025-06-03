#pragma once

#include <SDL2/SDL.h>
#include <cstdint>
#include <string>

class Texture {
 public:
  Texture(const std::string& image_file_path);
  ~Texture() noexcept { SDL_FreeSurface(surface_); }
  uint16_t GetWidth() const noexcept { return surface_->w; }
  uint16_t GetHeight() const noexcept { return surface_->h; }
  SDL_Surface* GetSurface() const noexcept { return surface_; }

 private:
  const std::string image_file_path_;
  SDL_Surface* surface_;
};
