#include "plane.hpp"

Plane::Plane(float x, float y, float z, float w) : vector_(x, y, z, w)
{
  assert(!(x == 0 && y == 0 && z == 0));
}

Plane::Plane(Eigen::Vector4f vector_4f) : vector_(vector_4f)
{
  assert(vector_4f({0, 1, 2}) != Eigen::Vector3f(0, 0, 0));
}

Eigen::Vector4f Plane::GetVector() const
{
  return vector_;
}

float Plane::DistanceFromOrigin() const
{
  return vector_[3] / vector_({0, 1, 2}).norm();
}