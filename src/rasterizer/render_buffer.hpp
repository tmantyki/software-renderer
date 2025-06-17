#pragma once

#include <cassert>
#include <cstdlib>
#include "geometry/common.hpp"

union Sample {
  struct ARGB {
    u8 alpha;
    u8 red;
    u8 green;
    u8 blue;
  } argb;
  u8 data[4];
};

static_assert(sizeof(Sample) == kNumberOfPixelChannels * sizeof(u8));

struct RenderBuffer {
  RenderBuffer(int pitch)
      : pixels(static_cast<Sample*>(
            std::aligned_alloc(kCacheLineSize,
                               sizeof(Sample) * pitch * kWindowHeight))),
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
  Sample* const pixels;
  f32* const z_buffer;
  const int pitch;
};