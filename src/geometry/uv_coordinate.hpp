#ifndef UV_COORDINATE_HPP
#define UV_COORDINATE_HPP

#include "common.hpp"

class UVCoordinate {
 public:
  UVCoordinate() noexcept;
  UVCoordinate(const float u, const float v) noexcept;
  UVCoordinate(const Vector2& vector) noexcept;
  Vector2 GetVector() const noexcept;
  Vector2 operator*(const float& float_val) const noexcept;
  Vector2 operator/(const float& float_val) const noexcept;

 private:
  Vector2 uv_;
};

#endif