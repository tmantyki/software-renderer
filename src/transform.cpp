#include "transform.hpp"
#include <Eigen/Geometry>

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
  translation_matrix_ = Eigen::Matrix4f::Identity();
  Eigen::Vector4f translation_vector = -camera_.GetLocation().GetVector();
  translation_vector(3) = 1;
  translation_matrix_.col(3) = translation_vector;

  // Rotation
  rotation_matrix_ = Eigen::Matrix4f::Identity();
  Eigen::Vector3f initial_direction(0, 0, 1);  // #TODO: is this correct?

  //// Pitch
  float cos_theta_expression = std::cos(camera_.GetPitch() / 2);
  float sin_theta_expression = std::sin(camera_.GetPitch() / 2);
  // #TODO: check signs below
  Eigen::Quaternionf quaternion_pitch(cos_theta_expression,
                                      sin_theta_expression, 0, 0);

  //// Yaw
  cos_theta_expression = std::cos(camera_.GetYaw() / 2);
  sin_theta_expression = std::sin(camera_.GetYaw() / 2);
  Eigen::Quaternionf quaternion_yaw(cos_theta_expression, 0,
                                    sin_theta_expression, 0);

  //// Roll
  cos_theta_expression = std::cos(camera_.GetRoll() / 2);
  sin_theta_expression = std::sin(camera_.GetRoll() / 2);
  Eigen::Vector3f direction_after_pitch_and_yaw =
      ((quaternion_yaw * quaternion_pitch) * initial_direction) *
      sin_theta_expression;
  Eigen::Quaternionf quaternion_roll(
      cos_theta_expression, direction_after_pitch_and_yaw(0),
      direction_after_pitch_and_yaw(1), direction_after_pitch_and_yaw(2));
  Eigen::Quaternionf quaternion_all =
      quaternion_roll * quaternion_yaw * quaternion_pitch;
  rotation_matrix_.block<3, 3>(0, 0) = quaternion_all.toRotationMatrix();
  matrix_ = translation_matrix_ * rotation_matrix_;
}