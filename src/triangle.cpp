#include "triangle.hpp"

Triangle::Triangle(Vertex vertex_1, Vertex vertex_2, Vertex vertex_3) : vertices_{vertex_1, vertex_2, vertex_3} {}
// #TODO: Triangle with zero area

Vertex Triangle::GetVertex(size_t index) const
{
  assert(index < 3);
  return vertices_[index];
}

TriangleArray::TriangleArray() : triangles_{nullptr}, used_(0) {}

std::array<Triangle *, kMaxTriangles> TriangleArray::GetArray() const
{
  return triangles_;
}

size_t TriangleArray::GetUsed() const
{
  return used_;
}

bool TriangleArray::AddTriangle(Triangle *triangle)
{
  if (used_ == kMaxTriangles)
    return false;
  triangles_[used_++] = triangle;
  return true;
}