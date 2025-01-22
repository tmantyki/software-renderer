#include "vertex.hpp"
#include "common.hpp"

Vertex::Vertex() : Vertex(0, 0, 0) {}

Vertex::Vertex(float x, float y, float z) : Point(x, y, z) {}

Vertex::Vertex(Vector3 vector) : Point(vector) {}

Vertex::Vertex(Vector4 vector) : Point(vector) {}

Vertex::Vertex(Point& point) : Point(point) {}

Vertex::Vertex(const InterpolatedVertex& iv) noexcept
    : Point(Vector4(iv.vectors[0] * (1 - iv.t) + iv.vectors[1] * iv.t)),
      uv_coordinate_(iv.uv[0] * (1 - iv.t) + iv.uv[1] * iv.t) {
  assert(iv.t >= 0 && iv.t <= 1);
}

Vertex::Vertex(const Vector4& vector,
               const UVCoordinate& uv_coordinate) noexcept
    : Point(vector), uv_coordinate_(uv_coordinate) {}

Vertex::Vertex(const Vertex& vertex, const UVCoordinate& uv_coordinate) noexcept
    : Point(vertex), uv_coordinate_(uv_coordinate) {}

const UVCoordinate Vertex::GetUVCoordinate() const noexcept {
  return uv_coordinate_;
}