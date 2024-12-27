#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "geometry/camera.hpp"

class Player : public Camera {
 public:
  Player();

 private:
  float speed_;
  Vector3 movement_direction_;
};

#endif