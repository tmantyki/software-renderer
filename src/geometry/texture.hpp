#pragma once

#include <SDL2/SDL.h>
#include <string>
#include "rasterizer/render_buffer.hpp"

class Texture {
 public:
  Texture(const std::string& image_file_path);
  ~Texture() noexcept { SDL_FreeSurface(surface_); }
  u32 GetWidth() const noexcept { return surface_->w; }
  u32 GetHeight() const noexcept { return surface_->h; }
  SDL_Surface* GetSurface() const noexcept { return surface_; }
  Pixel* GetTexels() const noexcept {
    return reinterpret_cast<Pixel*>(surface_->pixels);
  }

 private:
  const std::string image_file_path_;
  SDL_Surface* surface_;
};
