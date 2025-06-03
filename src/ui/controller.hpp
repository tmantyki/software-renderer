#pragma once

#include "geometry/common.hpp"

class Controller {
 public:
  Controller() noexcept
      : x_(AxisDirection::kNeutral),
        y_(AxisDirection::kNeutral),
        z_(AxisDirection::kNeutral),
        pitch_(AxisDirection::kNeutral),
        yaw_(AxisDirection::kNeutral),
        roll_(AxisDirection::kNeutral),
        quit_request_(false) {}
  void UpdateState() noexcept;
  bool CheckQuitRequest() const noexcept { return quit_request_; }
  bool ConsumeRasterizerToggleRequest() noexcept {
    if (swap_rasterizer_) {
      swap_rasterizer_ = false;
      return true;
    }
    return false;
  }
  AxisDirection GetX() const noexcept { return x_; }
  AxisDirection GetY() const noexcept { return y_; }
  AxisDirection GetZ() const noexcept { return z_; }
  AxisDirection GetPitch() const noexcept { return pitch_; }
  AxisDirection GetYaw() const noexcept { return yaw_; }
  AxisDirection GetRoll() const noexcept { return roll_; }

 private:
  AxisDirection x_;
  AxisDirection y_;
  AxisDirection z_;
  AxisDirection pitch_;
  AxisDirection yaw_;
  AxisDirection roll_;
  bool quit_request_;
  bool swap_rasterizer_;
};
