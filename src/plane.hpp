#ifndef PLANE_HPP
#define PLANE_HPP

#include <Eigen/Core>

class Plane
{
public:
  Plane() = delete;
  Plane(float x, float y, float z, float w);
  Plane(Eigen::Vector4f vector_4f);
  Eigen::Vector4f GetVector() const;
  Eigen::Vector4f GetVectorNormalized() const;
  float DistanceFromOrigin() const;

private:
  Eigen::Vector4f vector_;
  Eigen::Vector4f vector_normalized_;
  void NormalizeVector();
};

#endif