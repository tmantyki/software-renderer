#pragma once

#include "common.hpp"
#include "point.hpp"
#include "uv_coordinate.hpp"

typedef struct InterpolatedVertex {
  std::array<Vector4, 2> vectors;
  std::array<UVCoordinate, 2> uv;
  float t;
} InterpolatedVertex;

class Vertex : public Point {
 public:
  Vertex() : Vertex(0, 0, 0) {}
  Vertex(float x, float y, float z) : Point(x, y, z) {}
  Vertex(Vector3 vector) : Point(vector) {}
  Vertex(Vector4 vector) : Point(vector) {}
  Vertex(Point& point) : Point(point) {}
  Vertex(const Vector4& vector, const UVCoordinate& uv_coordinate) noexcept
      : Point(vector), uv_coordinate_(uv_coordinate) {}
  Vertex(const Vertex& vertex, const UVCoordinate& uv_coordinate) noexcept
      : Point(vertex), uv_coordinate_(uv_coordinate) {}
  Vertex(const InterpolatedVertex& iv) noexcept
      : Point(Vector4(iv.vectors[0] * (1 - iv.t) + iv.vectors[1] * iv.t)),
        uv_coordinate_(iv.uv[0] * (1 - iv.t) + iv.uv[1] * iv.t) {
    assert(iv.t >= 0 && iv.t <= 1);
  }
  const UVCoordinate GetUVCoordinate() const noexcept { return uv_coordinate_; }

 private:
  UVCoordinate uv_coordinate_;
};
