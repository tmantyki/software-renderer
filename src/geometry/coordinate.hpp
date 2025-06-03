#pragma once

#include <Eigen/Core>

#include "common.hpp"

class Coordinate {
 public:
  Coordinate() = delete;
  Coordinate(float x, float y, float z, float w) : vector_(x, y, z, w) {}
  Coordinate(Vector4 vector_4f) : vector_(vector_4f) {}
  Vector4 GetVector() const { return vector_; }
  Vector4 cross3(const Coordinate& rhs) const {
    return vector_.cross3(rhs.vector_);
  }
  float dot(const Coordinate& rhs) const noexcept {
    return vector_.dot(rhs.vector_);
  }
  float dot(const Vector4& rhs) const noexcept { return vector_.dot(rhs); }
  bool operator==(const Coordinate& rhs) const {
    return vector_ == rhs.vector_;
  }
  bool operator!=(const Coordinate& rhs) const {
    return vector_ != rhs.vector_;
  }

 protected:
  Vector4 vector_;
};
