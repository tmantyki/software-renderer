#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "point.hpp"

class Camera {
 public:
  Camera();
  Camera(Point location, float pitch = 0, float yaw = 0, float roll = 0);
  const Point& GetLocation() const;
  float GetPitch() const;
  float GetYaw() const;
  float GetRoll() const;

 private:
  Point location_;
  float pitch_, yaw_, roll_;
};

#endif