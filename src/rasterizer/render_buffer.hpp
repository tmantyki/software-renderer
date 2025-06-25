#pragma once

#include <cassert>
#include <cstdlib>
#include "geometry/common.hpp"

union Pixel {
  struct {
    u8 alpha;
    u8 red;
    u8 green;
    u8 blue;
  } argb;
  u32 value;

  Pixel() noexcept = default;

  Pixel(const u32 rhs) noexcept : value(rhs) {}

  Pixel(const u8 alpha, const u8 red, const u8 green, const u8 blue) noexcept
      : argb{alpha, red, green, blue} {}

  Pixel(const Pixel& rhs) noexcept : value(rhs.value) {}

  Pixel& operator=(const Pixel& rhs) noexcept {
    value = rhs.value;
    return *this;
  }

  Pixel& operator=(const u32 rhs) noexcept {
    value = rhs;
    return *this;
  }

  operator u32() noexcept { return value; }
};

static_assert(sizeof(Pixel) == kNumberOfPixelChannels * sizeof(u8));

struct RenderBuffer {
  RenderBuffer(int pitch)
      : pixels(static_cast<Pixel*>(
            std::aligned_alloc(kCacheLineSize,
                               sizeof(Pixel) * pitch * kWindowHeight))),
        z_buffer(static_cast<f32*>(
            std::aligned_alloc(kCacheLineSize,
                               sizeof(f32) * pitch * kWindowHeight))),
        pitch(pitch) {
    assert((pitch % kBytesPerPixel) == 0);
    assert(pitch == (kWindowHeight * kBytesPerPixel));
  }
  ~RenderBuffer() {
    std::free(pixels);
    std::free(z_buffer);
  }
  Pixel* const pixels;
  f32* const z_buffer;
  const int pitch;
};
