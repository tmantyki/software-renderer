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
  UpdateDirectionByKeyPair(SDL_SCANCODE_A, SDL_SCANCODE_D, y_, keyboard_state);
  UpdateDirectionByKeyPair(SDL_SCANCODE_S, SDL_SCANCODE_W, z_, keyboard_state);
  UpdateDirectionByKeyPair(SDL_SCANCODE_DOWN, SDL_SCANCODE_UP, pitch_,
                           keyboard_state);
  UpdateDirectionByKeyPair(SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, yaw_,
                           keyboard_state);
  quit_request_ = static_cast<bool>(keyboard_state[SDL_SCANCODE_ESCAPE]);
}

bool Controller::CheckQuitRequest() const {
  return quit_request_;
}

void Controller::OffsetCamera(Camera& camera) const {
  const float translation_offset = 0.01;
  Vector3 translation_vector = {static_cast<float>(x_), static_cast<float>(y_),
                                static_cast<float>(z_)};
  translation_vector *= translation_offset;
  camera.SetLocation(Vector3(camera.GetLocation().GetVector()({0, 1, 2}) +
                             translation_vector));
  std::cout << camera.GetLocation().GetVector() << "\n";
}