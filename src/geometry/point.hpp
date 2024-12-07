#ifndef POINT_HPP
#define POINT_HPP

#include "coordinate.hpp"
#include "direction.hpp"
#include "plane.hpp"

class Point : public Coordinate {
 public:
  Point() = delete;
  Point(float x, float y, float z);
  Point(float x, float y, float z, float w);
  Point(Vector3 vector);
  Point(Vector4 vector);
  float SignedDistanceFromPlane(const Plane& plane) const;
  Direction operator-(const Point& rhs) const;
};

#endif