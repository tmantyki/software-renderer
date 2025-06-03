#pragma once

#include "point.hpp"

class Camera {
 public:
  Camera() : Camera({0, 0, 0}) {}
  // #TODO: assert valid range [0, 2pi] rad
  Camera(Point location, float pitch = 0, float yaw = 0, float roll = 0)
      : location_(location), pitch_(pitch), yaw_(yaw), roll_(roll) {}
  const Point& GetLocation() const { return location_; }
  float GetPitch() const { return pitch_; }
  float GetYaw() const { return yaw_; }
  float GetRoll() const { return roll_; }
  void SetLocation(const Point& location) { location_ = location; }
  void SetPitch(float pitch) { pitch_ = pitch; }
  void SetYaw(float yaw) { yaw_ = yaw; }
  void SetRoll(float roll) { roll_ = roll; }
  bool operator==(const Camera& rhs) const;

 private:
  Point location_;
  float pitch_, yaw_, roll_;
};
