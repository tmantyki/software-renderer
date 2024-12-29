#include "point.hpp"
#include "common.hpp"

Point::Point(float x, float y, float z) : Coordinate(x, y, z, 1) {}

Point::Point(float x, float y, float z, float w) : Coordinate(x, y, z, w) {}

Point::Point(Vector3 vector) : Coordinate(vector[0], vector[1], vector[2], 1) {}

Point::Point(Vector4 vector) : Coordinate(vector) {}

float Point::SignedDistanceFromPlane(const Plane& plane) const {
  return vector_.dot(plane.GetVectorNormalized());
}

Direction Point::operator-(const Point& rhs) const {
  return Direction(Vector4(vector_ - rhs.vector_));
}
// #TODO: make operator part of coordinate.