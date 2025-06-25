#pragma once

#include <SDL2/SDL.h>
#include <string>
#include "rasterizer/render_buffer.hpp"

template <typename TilingPolicy>
class Texture {
 public:
  Texture(const std::string& image_file_path);
  ~Texture() noexcept { std::free(texels_); }
  i32 GetWidth() const noexcept { return width_; }
  i32 GetHeight() const noexcept { return height_; }
  Pixel* GetTexels() const noexcept { return texels_; }
  Pixel GetTexel(i32 x, i32 y) const noexcept {
    return TilingPolicy::GetTexel(x, y, texels_, width_);
  }

 private:
  const std::string image_file_path_;
  Pixel* texels_;
  i32 width_;
  i32 height_;
};

struct LinearTiling {
  static Pixel GetTexel(i32 x,
                        i32 y,
                        const Pixel* __restrict__ texels,
                        i32 stride) noexcept {
    return texels[y * stride + x];
  }
  static void CopyFromLinearSurface(Pixel* __restrict__ destination,
                                   const Pixel* __restrict__ source,
                                   i32 width,
                                   i32 height,
                                   i32 stride) {
    for (i32 y = 0; y < height; y++)
      for (i32 x = 0; x < width; x++)
        destination[y * width + x] = source[y * stride + x];
  }
};

template class Texture<LinearTiling>;