#pragma once

#include <Eigen/Core>
#include "camera.hpp"
#include "common.hpp"
#include "space.hpp"

class Transform {
 public:
  Transform() : matrix_(Matrix4::Zero()) {}
  Transform(Matrix4 matrix) : matrix_(matrix) {}
  Matrix4 GetMatrix() const noexcept { return matrix_; }
  virtual bool UpdateTransform() = 0;

 protected:
  Matrix4 matrix_;  // #TODO: rename to mapping?
};

class CameraTransform : public Transform {
 public:
  CameraTransform() : CameraTransform(Camera()) {}
  // reference arg instead?
  CameraTransform(Camera camera)
      : Transform(Matrix4::Identity()), camera_(camera) {
    UpdateTransform();
  }
  Camera& GetCamera() { return camera_; }
  bool UpdateTransform() noexcept override;
  Matrix4 GetMatrixInverse() const noexcept { return matrix_inverse_; }
  bool operator==(const CameraTransform& rhs) const {
    return this->camera_ == rhs.camera_;
  }

 private:
  Matrix4 matrix_inverse_;
  Camera camera_;
  Matrix4 rotation_matrix_;
  Matrix4 translation_matrix_;
};

class PerspectiveProjection : public Transform {
 public:
  PerspectiveProjection() = delete;
  PerspectiveProjection(float near,
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
  float GetNear() const noexcept { return near_; }
  float GetFar() const noexcept { return far_; }
  float GetLeft() const noexcept { return left_; }
  float GetRight() const noexcept { return right_; }
  float GetTop() const noexcept { return top_; }
  float GetBottom() const noexcept { return bottom_; }
  bool UpdateTransform() noexcept override;
  bool operator==(const PerspectiveProjection& rhs) const;

 private:
  float near_, far_, left_, right_, top_, bottom_;
};

class ViewportTransform : public Transform {
 public:
  ViewportTransform() = delete;
  ViewportTransform(uint16_t width,
                    uint16_t height,
                    int16_t x_offset = 0,
                    int16_t y_offset = 0)
      : width_(width),
        height_(height),
        x_offset_(x_offset),
        y_offset_(y_offset) {
    UpdateTransform();
  }
  uint16_t GetWidth() const noexcept { return width_; }
  uint16_t GetHeight() const noexcept { return height_; }
  int16_t GetOffsetX() const noexcept { return x_offset_; }
  int16_t GetOffsetY() const noexcept { return y_offset_; }
  bool UpdateTransform() noexcept override;
  bool operator==(const ViewportTransform& rhs) const;

 private:
  uint16_t width_, height_;
  int16_t x_offset_, y_offset_;
};

class TransformPipeline {
 public:
  TransformPipeline() = delete;
  TransformPipeline(CameraTransform& camera,
                    PerspectiveProjection& perspective,
                    ViewportTransform& viewport)
      : camera_(camera), perspective_(perspective), viewport_(viewport) {}
  CameraTransform& GetCameraTransform() { return camera_; }
  PerspectiveProjection& GetPerspectiveProjection() { return perspective_; }
  ViewportTransform& GetViewportTransform() { return viewport_; }
  void RunPipeline(const Space& input_space);
  const Space& GetOutputSpace() const { return output_space_; }

 private:
  CameraTransform& camera_;
  PerspectiveProjection& perspective_;
  ViewportTransform& viewport_;
  Space output_space_;
};
