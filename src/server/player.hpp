#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "geometry/camera.hpp"

class Player : public Camera {
 public:
  Player();

 private:
  float velocity_x_;
  float velocity_y_;
  float velocity_z_;
  float velocity_pitch_;
  float velocity_yaw_;
  float velocity_roll_;
};

#endif