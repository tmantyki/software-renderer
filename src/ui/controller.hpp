#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include "geometry/camera.hpp"
#include "geometry/common.hpp"


class Controller {
 public:
  Controller();
  void UpdateState();
  bool CheckQuitRequest() const;
  void OffsetCamera(Camera& camera) const;

 private:
  AxisDirection x_;
  AxisDirection y_;
  AxisDirection z_;
  AxisDirection pitch_;
  AxisDirection yaw_;
  AxisDirection roll_;
  bool quit_request_;
};

#endif