#include "transform.hpp"
#include <Eigen/Geometry>

// #TODO: use word mapping instead of transform or matrix
// #TODO: refactor!
bool CameraTransform::UpdateTransform() noexcept {
  // Translation
  translation_matrix_ = Matrix4::Identity();
  Vector4 translation_vector = -camera_.GetLocation().GetVector();
  translation_vector(3) = 1;
  translation_matrix_.col(3) = translation_vector;

  // Rotation
  rotation_matrix_ = Matrix4::Identity();

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
  matrix_ = rotation_matrix_ * translation_matrix_;
  matrix_inverse_ = matrix_.inverse();  // #TODO: faster to compute manually?
  return true;
}

bool PerspectiveProjection::UpdateTransform() noexcept {
  matrix_(0, 0) = 2 * near_ / (right_ - left_);
  matrix_(0, 2) = (right_ + left_) / (right_ - left_);
  matrix_(1, 1) = 2 * near_ / (top_ - bottom_);
  matrix_(1, 2) = (top_ + bottom_) / (top_ - bottom_);
  matrix_(2, 2) = -(far_ + near_) / (far_ - near_);
  matrix_(2, 3) = -(2 * near_ * far_) / (far_ - near_);
  matrix_(3, 2) = -1;
  return true;
}

bool PerspectiveProjection::operator==(const PerspectiveProjection& rhs) const {
  if (this->GetNear() != rhs.GetNear())
    return false;
  if (this->GetFar() != rhs.GetFar())
    return false;
  if (this->GetLeft() != rhs.GetLeft())
    return false;
  if (this->GetRight() != rhs.GetRight())
    return false;
  if (this->GetTop() != rhs.GetTop())
    return false;
  if (this->GetBottom() != rhs.GetBottom())
    return false;
  return true;
}

bool ViewportTransform::UpdateTransform() noexcept {
  matrix_ = Matrix4::Identity();
  // bit shift instead of division by 2
  matrix_(0, 0) = (width_ >> 1) - kViewportRoundingBias;
  matrix_(0, 3) = (width_ >> 1) + x_offset_;
  matrix_(1, 1) = -(height_ >> 1) + kViewportRoundingBias;
  matrix_(1, 3) = (height_ >> 1) + y_offset_;
  return true;
}

bool ViewportTransform::operator==(const ViewportTransform& rhs) const {
  if (this->GetWidth() != rhs.GetWidth())
    return false;
  if (this->GetHeight() != rhs.GetHeight())
    return false;
  if (this->GetOffsetX() != rhs.GetOffsetX())
    return false;
  if (this->GetOffsetY() != rhs.GetOffsetY())
    return false;
  return true;
}

void TransformPipeline::RunPipeline(const Space& input_space) {
  output_space_ = input_space;

  // Back-face culling
  const Vector4 location = camera_.GetCamera().GetLocation().GetVector();
  for (size_t t = 0; t < output_space_.GetTriangleCount(); t++) {
    const Vertex& vertex = output_space_.GetTriangles()[t]->GetVertex(0);
    const Vector4& normal = output_space_.GetTriangles()[t]->GetNormal();
    if ((vertex - location).dot(normal) <= 0)
      output_space_.EnqueueRemoveTriangle(t);
  }
  output_space_.UpdateSpace();

  // #TODO: optimize by giving as two arguments rather than ready product
  output_space_.TransformVertices(perspective_.GetMatrix() *
                                  camera_.GetMatrix());
  for (Axis axis : {Axis::kX, Axis::kY, Axis::kZ}) {
    for (AxisDirection axis_direction :
         {AxisDirection::kNegative, AxisDirection::kPositive}) {
      output_space_.ClipAllTriangles(axis, axis_direction);
    }
  }
  output_space_.Dehomogenize();
  output_space_.TransformVertices(viewport_.GetMatrix());
}
