#include <SDL2/SDL.h>
#include <iostream>

#include "ui.hpp"

UserInterface::UserInterface()
    : UserInterface(kCentered, kCentered, 800, 800) {}

UserInterface::UserInterface(int16_t x_position,
                             int16_t y_position,
                             uint16_t width,
                             uint16_t height)
    : x_position_(x_position),
      y_position_(y_position),
      width_(width),
      height_(height),
      window_created_(false) {}

bool UserInterface::CreateWindow() {
  if (SDL_Init(SDL_INIT_VIDEO))
    return false;
  return true;
}