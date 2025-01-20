#include "vertex.hpp"
#include "common.hpp"

Vertex::Vertex() : Vertex(0, 0, 0) {}

Vertex::Vertex(float x, float y, float z) : Point(x, y, z) {}

Vertex::Vertex(Vector3 vector) : Point(vector) {}

Vertex::Vertex(Vector4 vector) : Point(vector) {}

Vertex::Vertex(Point& point) : Point(point) {}

// #TODO: apply sorting logic?
Vertex::Vertex(const Vertex& vertex_a, const Vertex& vertex_b, float t) noexcept
    : Point((vertex_a.vector_ * (1 - t) + vertex_b.vector_ * t).eval()),
      uv_coordinate_(vertex_a.GetUVCoordinate() * (1 - t) +
                     vertex_b.GetUVCoordinate() * t) {
  assert(t >= 0 && t <= 1);
}

Vertex::Vertex(const Vertex& vertex, const UVCoordinate& uv_coordinate) noexcept
    : Point(vertex), uv_coordinate_(uv_coordinate) {}

const UVCoordinate Vertex::GetUVCoordinate() const noexcept {
  return uv_coordinate_;
}