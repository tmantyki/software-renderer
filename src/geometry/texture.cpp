#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_surface.h>
#include <cassert>
#include <cstddef>
#include <cstdlib>

#include "common.hpp"
#include "texture.hpp"



template <typename LayoutPolicy>
Texture<LayoutPolicy>::Texture(const std::string& image_file_path)
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
  if (!LayoutPolicy::LegalDimensions(width_, height_))
    throw std::runtime_error("Illegal texture dimensions!");
  assert(width_ % kTileLength == 0);
  assert(height_ % kTileLength == 0);
  const size_t alloc_size = LayoutPolicy::GetAllocationSize(width_, height_);
  texels_ =
      reinterpret_cast<Pixel*>(std::aligned_alloc(kCacheLineSize, alloc_size));
  LayoutPolicy::CopyFromLinearSurface(
      texels_, reinterpret_cast<Pixel*>(converted_surface->pixels), width_,
      height_, converted_surface->pitch / kBytesPerPixel);
  SDL_FreeSurface(converted_surface);
}
