#ifndef VERTEX_HPP
#define VERTEX_HPP

#include "common.hpp"
#include "point.hpp"

class Vertex : public Point {
 public:
  Vertex();
  Vertex(float x, float y, float z);
  Vertex(Vector3 vector);
  Vertex(Vector4 vector);
  Vertex(Point& point);
  Vertex(const Vertex& vertex_a, const Vertex& vertex_b, float t) noexcept;
  float GetCoordinateU() const noexcept;
  float GetCoordinateV() const noexcept;

   private:
    float u_;
    float v_;

};

#endif