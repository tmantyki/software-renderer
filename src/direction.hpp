#ifndef DIRECTION_HPP
#define DIRECTION_HPP

#include "coordinate.hpp"

class Direction : public Coordinate
{
public:
  Direction();
  Direction(float x, float y, float z);
  Direction(Eigen::Vector3f vector_3f);
};

#endif