#include "uv_coordinate.hpp"
#include "common.hpp"

UVCoordinate::UVCoordinate() noexcept : UVCoordinate(0, 0) {}

UVCoordinate::UVCoordinate(const float u, const float v) noexcept : uv_{u, v} {}

UVCoordinate::UVCoordinate(const Vector2& vector) noexcept : uv_(vector) {}

Vector2 UVCoordinate::GetVector() const noexcept {
  return uv_;
}

Vector2 UVCoordinate::operator*(const float& float_val) const noexcept {
  return float_val * uv_;
}