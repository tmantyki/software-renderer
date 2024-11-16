#include "triangle.hpp"
#include "common.hpp"

Triangle::Triangle(Vertex vertex_1, Vertex vertex_2, Vertex vertex_3)
    : vertices_{vertex_1, vertex_2, vertex_3} {}
// #TODO: Triangle with zero area

Vertex Triangle::GetVertex(size_t index) const {
  assert(index < 3);
  return vertices_[index];
}

TriangleArray::TriangleArray() : num_triangles_(0), triangles_{nullptr} {}

std::array<Triangle*, kMaxTriangles> TriangleArray::GetArray() const {
  return triangles_;
}

size_t TriangleArray::GetTriangleCount() const {
  return num_triangles_;
}

bool TriangleArray::AddTriangle(Triangle& triangle) {
  if (num_triangles_ == kMaxTriangles)
    return false;
  triangles_[num_triangles_++] = &triangle;
  return true;
}