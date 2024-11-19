#include "transform.hpp"

Transform::Transform(Eigen::Matrix4f matrix) : matrix_(matrix) {}

Eigen::Matrix4f Transform::GetMatrix() const {
  return matrix_;
}