#pragma once

#include "common.hpp"
#include "coordinate.hpp"

class Direction : public Coordinate {
 public:
  Direction() = delete;
  Direction(float x, float y, float z) : Coordinate(x, y, z, 0) {}
  Direction(Vector3 vector) : Coordinate(vector[0], vector[1], vector[2], 0) {}
  Direction(Vector4 vector) : Coordinate(vector) {}
};
