#include "point.hpp"

Point::Point() : Coordinate(0, 0, 0, 1) {}

Point::Point(float x, float y, float z) : Coordinate(x, y, z, 1) {}

Point::Point(Eigen::Vector3f vector) : Coordinate(vector[0], vector[1], vector[2], 1) {}

float Point::DistanceFromPlane(const Plane &plane) const
{
  return vector_.dot(plane.GetVectorNormalized());
}