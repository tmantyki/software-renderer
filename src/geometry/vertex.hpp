#ifndef VERTEX_HPP
#define VERTEX_HPP

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
  Vertex();
  Vertex(float x, float y, float z);
  Vertex(Vector3 vector);
  Vertex(Vector4 vector);
  Vertex(Point& point);
  Vertex(const Vector4& vector, const UVCoordinate& uv_coordinate) noexcept;
  Vertex(const Vertex& vertex, const UVCoordinate& uv_coordinate) noexcept;
  Vertex(const InterpolatedVertex& iv) noexcept;
  const UVCoordinate GetUVCoordinate() const noexcept;

 private:
  UVCoordinate uv_coordinate_;
};

#endif