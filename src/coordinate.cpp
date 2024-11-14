#include "coordinate.hpp"

Coordinate::Coordinate() : vector_(0, 0, 0, 0) {}

Coordinate::Coordinate(float x, float y, float z, float w) : vector_(x, y, z, w) {}

Coordinate::Coordinate(Eigen::Vector4f vector_4f) : vector_(vector_4f) {}

Eigen::Vector4f Coordinate::GetVector() const
{
  return vector_;
}

bool Coordinate::operator==(const Coordinate &rhs) const {
  return vector_ == rhs.vector_;
}

bool Coordinate::operator!=(const Coordinate &rhs) const {
  return vector_ != rhs.vector_;
}