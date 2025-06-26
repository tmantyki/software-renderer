#pragma once

#include <SDL2/SDL.h>
#include <string>
#include "geometry/common.hpp"
#include "rasterizer/render_buffer.hpp"

constexpr i32 kTileLength = 4;
constexpr i32 kTileSize = kTileLength * kTileLength;

template <typename LayoutPolicy>
class Texture {
 public:
  Texture(const std::string& image_file_path);
  ~Texture() noexcept { std::free(texels_); }
  i32 GetWidth() const noexcept { return width_; }
  i32 GetHeight() const noexcept { return height_; }
  Pixel* GetTexels() const noexcept { return texels_; }
  Pixel GetTexel(i32 x, i32 y) const noexcept {
    return LayoutPolicy::GetTexel(x, y, texels_, width_);
  }

 private:
  const std::string image_file_path_;
  Pixel* texels_;
  i32 width_;
  i32 height_;
};

struct LinearLayout {
  static size_t GetAllocationSize(i32 width, i32 height) noexcept {
    return width * height * kBytesPerPixel;
  }
  static Pixel GetTexel(i32 x,
                        i32 y,
                        const Pixel* __restrict__ texels,
                        i32 width) noexcept {
    return texels[y * width + x];
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
struct TiledLayout {
  static size_t GetAllocationSize(i32 width, i32 height) noexcept {
    const i32 tile_x_count = ((width - 1) / kTileLength) + 1;
    const i32 tile_y_count = ((height - 1) / kTileLength) + 1;
    return tile_x_count * tile_y_count * kTileSize * kBytesPerPixel;
  }

  static i32 GetTexelIndex(i32 x, i32 y, i32 width) noexcept {
    const i32 tile_x_count = (static_cast<u32>(width - 1) / kTileLength) + 1;
    const i32 tile_x = static_cast<u32>(x) / kTileLength;
    const i32 tile_y = static_cast<u32>(y) / kTileLength;
    const i32 inner_x = static_cast<u32>(x) % kTileLength;
    const i32 inner_y = static_cast<u32>(y) % kTileLength;
    const i32 index =
        static_cast<u32>((static_cast<u32>(tile_y) * tile_x_count) + tile_x) *
            kTileSize +
        static_cast<u32>(inner_y) * kTileLength + inner_x;
    return index;
  }

  static Pixel GetTexel(i32 x,
                        i32 y,
                        const Pixel* __restrict__ texels,
                        i32 width) noexcept {
    return texels[GetTexelIndex(x, y, width)];
  }
  static void CopyFromLinearSurface(Pixel* __restrict__ destination,
                                    const Pixel* __restrict__ source,
                                    i32 width,
                                    i32 height,
                                    i32 stride) {
    for (i32 y = 0; y < height; y++)
      for (i32 x = 0; x < width; x++)
        destination[GetTexelIndex(x, y, width)] = source[y * stride + x];
  }
};

template class Texture<TiledLayout>;