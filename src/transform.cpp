#include "transform.hpp"
#include <Eigen/Geometry>

Transform::Transform() : matrix_(Eigen::Matrix4f::Zero()) {};

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
// #TODO: refactor!
void CameraTransform::UpdateTransformFromCamera() {
  // Translation
  translation_matrix_ = Eigen::Matrix4f::Identity();
  Eigen::Vector4f translation_vector = -camera_.GetLocation().GetVector();
  translation_vector(3) = 1;
  translation_matrix_.col(3) = translation_vector;

  // Rotation
  rotation_matrix_ = Eigen::Matrix4f::Identity();

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
  Eigen::Quaternionf quaternion_roll(cos_theta_expression, 0, 0,
                                     sin_theta_expression);
  Eigen::Quaternionf quaternion_all =
      quaternion_roll * quaternion_pitch * quaternion_yaw;
  rotation_matrix_.block<3, 3>(0, 0) = quaternion_all.toRotationMatrix();
  matrix_ = translation_matrix_ * rotation_matrix_;
}

PerspectiveProjection::PerspectiveProjection(float near,
                                             float far,
                                             float left,
                                             float right,
                                             float top,
                                             float bottom)
    : Transform(),
      near_(near),
      far_(far),
      left_(left),
      right_(right),
      top_(top),
      bottom_(bottom) {
  assert(near >= 0 && far > 0);
  assert(near < far);
  assert(left < right);
  assert(top > bottom);
  UpdateTransformFromParameters();
}

float PerspectiveProjection::GetNear() const {
  return near_;
}
float PerspectiveProjection::GetFar() const {
  return far_;
}
float PerspectiveProjection::GetLeft() const {
  return left_;
}
float PerspectiveProjection::GetRight() const {
  return right_;
}
float PerspectiveProjection::GetTop() const {
  return top_;
}
float PerspectiveProjection::GetBottom() const {
  return bottom_;
}

void PerspectiveProjection::UpdateTransformFromParameters() {
  matrix_(0, 0) = 2 * near_ / (right_ - left_);
  matrix_(0, 2) = (right_ + left_) / (right_ - left_);
  matrix_(1, 1) = 2 * near_ / (top_ - bottom_);
  matrix_(1, 2) = (top_ + bottom_) / (top_ - bottom_);
  matrix_(2, 2) = -(far_ + near_) / (far_ - near_);
  matrix_(2, 3) = -(2 * near_ * far_) / (far_ - near_);
  matrix_(3, 2) = -1;
}

ViewportTransformation::ViewportTransformation(uint16_t width,
                                               uint16_t height,
                                               int16_t x_offset,
                                               int16_t y_offset)
    : width_(width),
      height_(height),
      x_offset_(x_offset),
      y_offset_(y_offset) {}