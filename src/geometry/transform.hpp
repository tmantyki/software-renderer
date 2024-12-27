#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <Eigen/Core>
#include "camera.hpp"
#include "common.hpp"
#include "point.hpp"
#include "space.hpp"

class Transform {
 public:
  Transform();
  Transform(Matrix4 matrix);
  Matrix4 GetMatrix() const noexcept;
  virtual bool UpdateTransform() = 0;

 protected:
  Matrix4 matrix_;  // #TODO: rename to mapping?
};

class CameraTransform : public Transform {
 public:
  CameraTransform();
  CameraTransform(Camera camera);
  Camera& GetCamera();
  bool UpdateTransform() noexcept override;
  Matrix4 GetMatrixInverse() const noexcept;
  const Camera& GetCamera() const noexcept;
  bool operator==(const CameraTransform& rhs) const;

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
                        float bottom);
  float GetNear() const;
  float GetFar() const;
  float GetLeft() const;
  float GetRight() const;
  float GetTop() const;
  float GetBottom() const;
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
                    int16_t y_offset = 0);
  uint16_t GetWidth() const;
  uint16_t GetHeight() const;
  int16_t GetOffsetX() const;
  int16_t GetOffsetY() const;
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
                    ViewportTransform& viewport);
  CameraTransform& GetCameraTransform();
  PerspectiveProjection& GetPerspectiveProjection();
  ViewportTransform& GetViewportTransform();
  void RunPipeline(const Space& input_space);
  const Space& GetOutputSpace() const;

 private:
  CameraTransform& camera_;
  PerspectiveProjection& perspective_;
  ViewportTransform& viewport_;
  Space output_space_;
};

#endif