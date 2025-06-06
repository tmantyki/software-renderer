#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "common.hpp"
#include "texture.hpp"

Texture::Texture(const std::string& image_file_path)
    : image_file_path_(image_file_path), surface_(nullptr) {
  SDL_Surface* loaded_surface = IMG_Load(image_file_path_.c_str());
  if (!loaded_surface)  // #TODO: raise exception?
    return;
  surface_ = SDL_ConvertSurfaceFormat(loaded_surface, kDefaultPixelFormat, 0);
  if (!surface_) {
    // #TODO: raise exception?
  }
  SDL_FreeSurface(loaded_surface);
}

Texture::~Texture() noexcept {
  SDL_FreeSurface(surface_);
}

uint16_t Texture::GetWidth() const noexcept {
  return surface_->w;
}

uint16_t Texture::GetHeight() const noexcept {
  return surface_->h;
}

SDL_Surface* Texture::GetSurface() const noexcept {
  return surface_;
}