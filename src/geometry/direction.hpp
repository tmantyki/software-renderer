#ifndef DIRECTION_HPP
#define DIRECTION_HPP

#include "common.hpp"
#include "coordinate.hpp"

class Direction : public Coordinate {
 public:
  Direction() = delete;
  Direction(float x, float y, float z);
  Direction(Vector3 vector);
};

#endif