#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include "plane.hpp"
#include "vertex.hpp"

class Triangle
{
public:
  Triangle(Vertex vertex_1, Vertex vertex_2, Vertex vertex_3);
  Vertex GetVertex(size_t index) const;

private:
  Vertex vertices_[3];
};

#endif