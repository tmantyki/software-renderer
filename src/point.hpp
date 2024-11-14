#ifndef POINT_HPP
#define POINT_HPP

#include "coordinate.hpp"
#include "plane.hpp"

class Point : public Coordinate
{
public:
  Point();
  Point(float x, float y, float z);
  Point(Eigen::Vector3f vector_3f);
  float DistanceFromPlane(const Plane &plane) const;
};

#endif