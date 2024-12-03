#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <Eigen/Core>
#include "camera.hpp"
#include "common.hpp"
#include "point.hpp"

class Transform {
 public:
  Transform();
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

 private:
  float near_, far_, left_, right_, top_, bottom_;
  void UpdateTransformFromParameters();
};

class ViewportTransformation : public Transform {
 public:
  ViewportTransformation() = delete;
  ViewportTransformation(uint16_t width,
                         uint16_t height,
                         int16_t x_offset = 0,
                         int16_t y_offset = 0);

 private:
  uint16_t width_, height_;
  int16_t x_offset_, y_offset_;
};

class TransformPipeline {
 public:
  void UpdateCamera();
  void UpdatePerspective();

 private:
  CameraTransform camera_transform_;
  PerspectiveProjection perpective_projection_;
  Transform world_to_clip_transform_;
  Transform clip_to_viewspace_;
};

#endif