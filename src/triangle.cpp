#include "triangle.hpp"

Triangle::Triangle(Vertex vertex_1, Vertex vertex_2, Vertex vertex_3) : vertices_{vertex_1, vertex_2, vertex_3}
{
}

Vertex Triangle::GetVertex(size_t index) const {
  assert(index < 3);
  return vertices_[index];
}