#pragma once

#include "coordinate.hpp"
#include "direction.hpp"
#include "plane.hpp"

class Point : public Coordinate {
 public:
  Point() = delete;
  Point(float x, float y, float z) : Coordinate(x, y, z, 1) {}
  Point(float x, float y, float z, float w) : Coordinate(x, y, z, w) {}
  Point(Vector3 vector) : Coordinate(vector[0], vector[1], vector[2], 1) {}
  Point(Vector4 vector) : Coordinate(vector) {}
  float SignedDistanceFromPlane(const Plane& plane) const {
    return vector_.dot(plane.GetVectorNormalized());
  }
  Direction operator-(const Point& rhs) const {
    return Direction(Vector4(vector_ - rhs.vector_));
  }
  // #TODO: make operator part of coordinate.
};
