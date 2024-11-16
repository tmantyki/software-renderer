#include "plane.hpp"

Plane::Plane(float x, float y, float z, float w) : vector_(x, y, z, w) {
  assert(!(x == 0 && y == 0 && z == 0));
  NormalizeVector();
}

Plane::Plane(Eigen::Vector4f vector_4f) : vector_(vector_4f) {
  assert(vector_4f({0, 1, 2}) != Eigen::Vector3f(0, 0, 0));
  NormalizeVector();
}

void Plane::NormalizeVector() {
  vector_normalized_ = vector_;
  vector_normalized_[3] = 0;
  float norm = vector_normalized_.norm();
  vector_normalized_ /= norm;
  vector_normalized_[3] = vector_[3] / norm;
}

Eigen::Vector4f Plane::GetVector() const {
  return vector_;
}

Eigen::Vector4f Plane::GetVectorNormalized() const {
  return vector_normalized_;
}

float Plane::SignedDistanceFromOrigin() const {
  return Eigen::Vector4f(0, 0, 0, 1).dot(vector_normalized_);
}