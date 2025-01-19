#include "uv_coordinate.hpp"

UVCoordinate::UVCoordinate() noexcept : UVCoordinate(0, 0) {}

UVCoordinate::UVCoordinate(const float u, const float v) noexcept
    : u_(u), v_(v) {}

float UVCoordinate::GetCoordinateU() const noexcept {
  return u_;
}

float UVCoordinate::GetCoordinateV() const noexcept {
  return v_;
}