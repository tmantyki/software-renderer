#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <string>

class Texture {
 public:
  Texture(const std::string& image_file_path);
  ~Texture() noexcept;
  uint16_t GetWidth() const noexcept;
  uint16_t GetHeight() const noexcept;
  SDL_Surface* GetSurface() const noexcept;

 private:
  const std::string image_file_path_;
  SDL_Surface* surface_;
};

#endif