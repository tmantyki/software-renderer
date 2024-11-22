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

 protected:
  Eigen::Matrix4f matrix_;
};

class CameraTransform : public Transform {
 public:
  CameraTransform();
  CameraTransform(Camera camera);
  Camera& GetCamera();

 private:
  Camera camera_;
  Eigen::Matrix4f rotation_matrix_;
  Eigen::Matrix4f translation_matrix_;
  void UpdateTransformFromCamera();
};

#endif