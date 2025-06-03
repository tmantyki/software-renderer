#pragma once

#include "common.hpp"

class UVCoordinate {
 public:
  UVCoordinate() noexcept : UVCoordinate(0, 0) {}
  UVCoordinate(const float u, const float v) noexcept : uv_{u, v} {}
  UVCoordinate(const Vector2& vector) noexcept : uv_(vector) {}
  Vector2 GetVector() const noexcept { return uv_; }
  Vector2 operator*(const float& float_val) const noexcept {
    return uv_ * float_val;
  }
  Vector2 operator/(const float& float_val) const noexcept {
    return uv_ / float_val;
  }

 private:
  Vector2 uv_;
};
