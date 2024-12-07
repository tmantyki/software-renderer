#ifndef VERTEX_HPP
#define VERTEX_HPP

#include "common.hpp"
#include "point.hpp"

class Vertex : public Point {
 public:
  Vertex() = delete;
  Vertex(float x, float y, float z);
  Vertex(Vector3 vector);
  Vertex(Vector4 vector);
  Vertex(Point& point);
  // Vector3 GetAttributeColor() const;

  //  private:
  //   Vector3 attribute_color_;
};

#endif