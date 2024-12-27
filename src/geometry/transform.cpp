#include "transform.hpp"
#include <Eigen/Geometry>

Transform::Transform() : matrix_(Matrix4::Zero()) {};

Transform::Transform(Matrix4 matrix) : matrix_(matrix) {}

Matrix4 Transform::GetMatrix() const noexcept {
  return matrix_;
}

CameraTransform::CameraTransform() : CameraTransform(Camera()) {}

CameraTransform::CameraTransform(Camera camera)
    : Transform(Matrix4::Identity()), camera_(camera) {
  UpdateTransform();
}

Camera& CameraTransform::GetCamera() {
  return camera_;
}

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
  matrix_inverse_ = matrix_.inverse(); // #TODO: faster to compute manually?
  return true;
}

Matrix4 CameraTransform::GetMatrixInverse() const noexcept {
  return matrix_inverse_;
}

const Camera& CameraTransform::GetCamera() const noexcept {
  return camera_;
}

bool CameraTransform::operator==(const CameraTransform& rhs) const {
  return this->GetCamera() == rhs.GetCamera();
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
  UpdateTransform();
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

ViewportTransform::ViewportTransform(uint16_t width,
                                     uint16_t height,
                                     int16_t x_offset,
                                     int16_t y_offset)
    : width_(width), height_(height), x_offset_(x_offset), y_offset_(y_offset) {
  UpdateTransform();
}

uint16_t ViewportTransform::GetWidth() const {
  return width_;
}
uint16_t ViewportTransform::GetHeight() const {
  return height_;
}
int16_t ViewportTransform::GetOffsetX() const {
  return x_offset_;
}
int16_t ViewportTransform::GetOffsetY() const {
  return y_offset_;
}

bool ViewportTransform::UpdateTransform() noexcept {
  matrix_ = Matrix4::Identity();
  matrix_(0, 0) = width_ / 2;
  matrix_(0, 3) = (width_ / 2) + x_offset_;
  matrix_(1, 1) = -height_ / 2;
  matrix_(1, 3) = height_ / 2 + y_offset_;
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

TransformPipeline::TransformPipeline(CameraTransform& camera,
                                     PerspectiveProjection& perspective,
                                     ViewportTransform& viewport)
    : camera_(camera), perspective_(perspective), viewport_(viewport) {}

CameraTransform& TransformPipeline::GetCameraTransform() {
  return camera_;
}

PerspectiveProjection& TransformPipeline::GetPerspectiveProjection() {
  return perspective_;
}

ViewportTransform& TransformPipeline::GetViewportTransform() {
  return viewport_;
}

void TransformPipeline::RunPipeline(const Space& input_space) {
  output_space_ = input_space;
  // #TODO: optimize by giving as two arguments rather than ready product
  output_space_.TransformVertices(perspective_.GetMatrix() *
                                  camera_.GetMatrix());
  output_space_.TransformNormals(camera_.GetMatrix());  // check for correctness
  for (Axis axis : {Axis::kX, Axis::kY, Axis::kZ}) {
    for (AxisDirection axis_direction :
         {AxisDirection::kNegative, AxisDirection::kPositive}) {
      output_space_.ClipAllTriangles(axis, axis_direction);
    }
  }
  output_space_.Dehomogenize();
  output_space_.TransformVertices(viewport_.GetMatrix());
}

const Space& TransformPipeline::GetOutputSpace() const {
  return output_space_;
}