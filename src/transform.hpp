#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <Eigen/Core>
#include "camera.hpp"
#include "common.hpp"
#include "point.hpp"

class Transform {
 public:
  Transform() = delete;
  Transform(Eigen::Matrix4f matrix);
  Eigen::Matrix4f GetMatrix() const;

 private:
  Eigen::Matrix4f matrix_;
};

#endif