#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include <array>
#include "common.hpp"
#include "vertex.hpp"

// #TODO: implement vertex reuse

class Triangle {
 public:
  Triangle(Vertex vertex_1, Vertex vertex_2, Vertex vertex_3);
  Vertex GetVertex(size_t index) const;
  const Eigen::Vector4f& GetNormal() const;

 private:
  Vertex vertices_[3];
  Eigen::Vector4f normal_;
};

#endif