#pragma once

#include "geometry/camera.hpp"

class Player : public Camera {
 public:
  Player() noexcept
      // : Camera({0, 0, 0}), speed_(0), movement_direction_(Vector3::Zero()) {}
      : Camera({0, 0, 2.9}), speed_(0), movement_direction_(Vector3::Zero()) {
    (void)speed_;  // #TODO: remove
  }

 private:
  float speed_;
  Vector3 movement_direction_;
};
