#ifndef PLANE_HPP
#define PLANE_HPP

#include <Eigen/Core>

#include "common.hpp"

class Plane {
 public:
  Plane() = delete;
  Plane(float x, float y, float z, float w);
  Plane(Vector4 vector_4f);
  Vector4 GetVector() const;
  Vector4 GetVectorNormalized() const;
  float SignedDistanceFromOrigin() const;

 private:
  Vector4 vector_;
  Vector4 vector_normalized_;
  void NormalizeVector();
};

#endif