#include "camera.hpp"

Camera::Camera() : Camera({0, 0, 0}) {}

// #TODO: assert valid range [0, 2pi] rad
Camera::Camera(Point location, float pitch, float yaw, float roll)
    : location_(location), pitch_(pitch), yaw_(yaw), roll_(roll) {}

const Point& Camera::GetLocation() const {
  return location_;
}

float Camera::GetPitch() const {
  return pitch_;
}

float Camera::GetYaw() const {
  return yaw_;
}

float Camera::GetRoll() const {
  return roll_;
}