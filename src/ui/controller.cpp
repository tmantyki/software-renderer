#include <SDL2/SDL.h>

#include "controller.hpp"

namespace {
void UpdateDirectionByKeyPair(SDL_Scancode negative_scancode,
                              SDL_Scancode positive_scancode,
                              AxisDirection& movement_axis,
                              const uint8_t* keyboard_state) {
  if (keyboard_state[negative_scancode] == keyboard_state[positive_scancode])
    movement_axis = AxisDirection::kNeutral;
  else if (keyboard_state[negative_scancode])
    movement_axis = AxisDirection::kNegative;
  else if (keyboard_state[positive_scancode])
    movement_axis = AxisDirection::kPositive;
}
}  // namespace

Controller::Controller()
    : x_(AxisDirection::kNeutral),
      y_(AxisDirection::kNeutral),
      z_(AxisDirection::kNeutral),
      pitch_(AxisDirection::kNeutral),
      yaw_(AxisDirection::kNeutral),
      roll_(AxisDirection::kNeutral),
      quit_request_(false) {}

void Controller::UpdateState() {
  SDL_PumpEvents();
  const uint8_t* keyboard_state = SDL_GetKeyboardState(nullptr);
  UpdateDirectionByKeyPair(SDL_SCANCODE_A, SDL_SCANCODE_D, x_, keyboard_state);
  UpdateDirectionByKeyPair(SDL_SCANCODE_LCTRL, SDL_SCANCODE_SPACE, y_,
                           keyboard_state);
  UpdateDirectionByKeyPair(SDL_SCANCODE_W, SDL_SCANCODE_S, z_, keyboard_state);
  UpdateDirectionByKeyPair(SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, pitch_,
                           keyboard_state);
  UpdateDirectionByKeyPair(SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, yaw_,
                           keyboard_state);
  UpdateDirectionByKeyPair(SDL_SCANCODE_Q, SDL_SCANCODE_E, roll_,
                           keyboard_state);
  quit_request_ = static_cast<bool>(keyboard_state[SDL_SCANCODE_ESCAPE]);
}

bool Controller::CheckQuitRequest() const {
  return quit_request_;
}

AxisDirection Controller::GetX() const noexcept {
  return x_;
}

AxisDirection Controller::GetY() const noexcept {
  return y_;
}

AxisDirection Controller::GetZ() const noexcept {
  return z_;
}

AxisDirection Controller::GetPitch() const noexcept {
  return pitch_;
}

AxisDirection Controller::GetYaw() const noexcept {
  return yaw_;
}

AxisDirection Controller::GetRoll() const noexcept {
  return roll_;
}