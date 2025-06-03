#pragma once

#include <Eigen/Core>

#include "common.hpp"

class Plane {
 public:
  Plane() = delete;
  Plane(float x, float y, float z, float w) : vector_(x, y, z, w) {
    assert(!(x == 0 && y == 0 && z == 0));
    NormalizeVector();
  }
  Plane(Vector4 vector_4f) : vector_(vector_4f) {
    assert(vector_4f({0, 1, 2}) != Vector3(0, 0, 0));
    NormalizeVector();
  }
  Vector4 GetVector() const { return vector_; }
  Vector4 GetVectorNormalized() const { return vector_normalized_; }
  float SignedDistanceFromOrigin() const {
    return Vector4(0, 0, 0, 1).dot(vector_normalized_);
  }

 private:
  Vector4 vector_;
  Vector4 vector_normalized_;
  void NormalizeVector();
};
