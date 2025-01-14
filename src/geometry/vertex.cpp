#include "vertex.hpp"
#include "common.hpp"

Vertex::Vertex() : Vertex(0, 0, 0) {}

Vertex::Vertex(float x, float y, float z) : Point(x, y, z) {}

Vertex::Vertex(Vector3 vector) : Point(vector) {}

Vertex::Vertex(Vector4 vector) : Point(vector) {}

Vertex::Vertex(Point& point) : Point(point) {}

Vertex::Vertex(const Vertex& vertex_a, const Vertex& vertex_b, float t) noexcept
    : Point((vertex_a.vector_ * (1 - t) + vertex_b.vector_ * t).eval()),
      u_(vertex_a.GetCoordinateU() * (1 - t) + vertex_b.GetCoordinateU() * t),
      v_(vertex_a.GetCoordinateV() * (1 - t) + vertex_b.GetCoordinateV() * t) {
  assert(t >= 0 && t <= 1);
}

float Vertex::GetCoordinateU() const noexcept {
  return u_;
}
float Vertex::GetCoordinateV() const noexcept {
  return v_;
}