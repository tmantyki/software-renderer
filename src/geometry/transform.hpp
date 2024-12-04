#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <Eigen/Core>
#include "camera.hpp"
#include "common.hpp"
#include "space.hpp"
#include "point.hpp"

class Transform {
 public:
  Transform();
  Transform(Eigen::Matrix4f matrix);
  Eigen::Matrix4f GetMatrix() const;
  virtual bool UpdateTransform() = 0;

 protected:
  Eigen::Matrix4f matrix_;  // #TODO: rename to mapping?
};

class CameraTransform : public Transform {
 public:
  CameraTransform();
  CameraTransform(Camera camera);
  Camera& GetCamera();
  bool UpdateTransform() override;

 private:
  Camera camera_;
  Eigen::Matrix4f rotation_matrix_;
  Eigen::Matrix4f translation_matrix_;
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
  bool UpdateTransform() override;

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
  bool UpdateTransform() override;

 private:
  uint16_t width_, height_;
  int16_t x_offset_, y_offset_;
};

class TransformPipeline {
 public:
  TransformPipeline() = delete;
  TransformPipeline(std::shared_ptr<CameraTransform> camera,
                    std::shared_ptr<PerspectiveProjection> perspective,
                    std::shared_ptr<ViewportTransform> viewport);
  void UpdateOutput(Space& input_space);

 private:
  std::shared_ptr<CameraTransform> camera_;
  std::shared_ptr<PerspectiveProjection> perspective_;
  std::shared_ptr<ViewportTransform> viewport_;
  Space output_space_;
};

#endif