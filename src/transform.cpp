#include "transform.hpp"

Transform::Transform(Eigen::Matrix4f matrix) : matrix_(matrix) {}

Eigen::Matrix4f Transform::GetMatrix() const {
  return matrix_;
}

CameraTransform::CameraTransform() : CameraTransform(Camera()) {}

CameraTransform::CameraTransform(Camera camera)
    : Transform(Eigen::Matrix4f::Identity()), camera_(camera) {
  UpdateTransformFromCamera();
}

Camera& CameraTransform::GetCamera() {
  return camera_;
}

// #TODO: use word mapping instead of transform or matrix
void CameraTransform::UpdateTransformFromCamera() {
  // Translation
  Eigen::Vector4f translation_vector = -camera_.GetLocation().GetVector();
  translation_vector(3) = 1;
  matrix_.col(3) = translation_vector;
}