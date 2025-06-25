#pragma once

#include <SDL2/SDL.h>
#include <string>
#include "rasterizer/render_buffer.hpp"

class Texture {
 public:
  Texture(const std::string& image_file_path);
  ~Texture() noexcept {
    std::free(texels_);
  }
  i32 GetWidth() const noexcept { return width_; }
  i32 GetHeight() const noexcept { return height_; }
  Pixel* GetTexels() const noexcept { return texels_; }

 private:
  const std::string image_file_path_;
  Pixel* texels_;
  i32 width_;
  i32 height_;
};
