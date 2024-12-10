#ifndef COORDINATE_HPP
#define COORDINATE_HPP

#include <Eigen/Core>

#include "common.hpp"

class Coordinate {
 public:
  Coordinate() = delete;
  Coordinate(float x, float y, float z, float w);
  Coordinate(Vector4 vector_4f);
  Vector4 GetVector() const;
  Vector4 cross3(const Coordinate& rhs) const;
  float dot(const Coordinate& rhs) const;
  bool operator==(const Coordinate& rhs) const;
  bool operator!=(const Coordinate& rhs) const;

 protected:
  Vector4 vector_;
};

#endif