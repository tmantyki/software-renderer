#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include <array>
#include "plane.hpp"
#include "vertex.hpp"

constexpr size_t kMaxTriangles = 128;

class Triangle
{
public:
  Triangle(Vertex vertex_1, Vertex vertex_2, Vertex vertex_3);
  Vertex GetVertex(size_t index) const;

private:
  Vertex vertices_[3];
};

class TriangleArray
{
public:
  TriangleArray();
  std::array<Triangle *, kMaxTriangles> GetArray() const;
  size_t GetUsed() const;
  bool AddTriangle(Triangle* triangle);

private:
  std::array<Triangle *, kMaxTriangles> triangles_;
  size_t used_;
};

#endif