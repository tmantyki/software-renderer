#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_surface.h>
#include <cstddef>
#include <cstdlib>

#include "common.hpp"
#include "texture.hpp"

template <typename TilingPolicy>
Texture<TilingPolicy>::Texture(const std::string& image_file_path)
    : image_file_path_(image_file_path) {
  SDL_Surface* loaded_surface = IMG_Load(image_file_path_.c_str());
  if (!loaded_surface)  // #TODO: raise exception?
    return;
  SDL_Surface* converted_surface =
      SDL_ConvertSurfaceFormat(loaded_surface, kDefaultPixelFormat, 0);
  SDL_FreeSurface(loaded_surface);
  if (!converted_surface) {
    // #TODO: raise exception?
  }
  width_ = converted_surface->w;
  height_ = converted_surface->h;
  const i32 texture_size = width_ * height_;
  texels_ = reinterpret_cast<Pixel*>(
      std::aligned_alloc(kCacheLineSize, texture_size * kBytesPerPixel));
  TilingPolicy::CopyFromLinearSurface(
      texels_, reinterpret_cast<Pixel*>(converted_surface->pixels), width_,
      height_, converted_surface->pitch / kBytesPerPixel);
  SDL_FreeSurface(converted_surface);
}
