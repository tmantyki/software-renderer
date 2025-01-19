#ifndef VERTEX_HPP
#define VERTEX_HPP

#include "common.hpp"
#include "point.hpp"
#include "uv_coordinate.hpp"

class Vertex : public Point {
 public:
  Vertex();
  Vertex(float x, float y, float z);
  Vertex(Vector3 vector);
  Vertex(Vector4 vector);
  Vertex(Point& point);
  Vertex(const Vertex& vertex, const UVCoordinate& uv_coordinate) noexcept;
  Vertex(const Vertex& vertex_a, const Vertex& vertex_b, float t) noexcept;
  const UVCoordinate GetUVCoordinate() const noexcept;

 private:
  UVCoordinate uv_coordinate_;
};

#endif