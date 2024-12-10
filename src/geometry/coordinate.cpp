#include <Eigen/Geometry>

#include "common.hpp"
#include "coordinate.hpp"

Coordinate::Coordinate(float x, float y, float z, float w)
    : vector_(x, y, z, w) {}

Coordinate::Coordinate(Vector4 vector_4f) : vector_(vector_4f) {}

Vector4 Coordinate::GetVector() const {
  return vector_;
}

Vector4 Coordinate::cross3(const Coordinate& rhs) const {
  return vector_.cross3(rhs.vector_);
}

float Coordinate::dot(const Coordinate& rhs) const {
  return vector_.dot(rhs.vector_);
}

bool Coordinate::operator==(const Coordinate& rhs) const {
  return vector_ == rhs.vector_;
}

bool Coordinate::operator!=(const Coordinate& rhs) const {
  return vector_ != rhs.vector_;
}