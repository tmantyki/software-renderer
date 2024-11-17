#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include <array>
#include "common.hpp"
#include "plane.hpp"
#include "vertex.hpp"

// #TODO: implement vertex reuse

class Triangle {
 public:
  Triangle(Vertex vertex_1, Vertex vertex_2, Vertex vertex_3);
  Vertex GetVertex(size_t index) const;

 private:
  Vertex vertices_[3];
};

#endif