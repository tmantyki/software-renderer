#include <Eigen/Geometry>

#include "common.hpp"
#include "triangle.hpp"

Triangle::Triangle(Vertex vertex_1, Vertex vertex_2, Vertex vertex_3)
    : vertices_{vertex_1, vertex_2, vertex_3},
      normal_((vertex_2.GetVector() - vertex_1.GetVector())
                  .cross3(vertex_3.GetVector() - vertex_1.GetVector())) {}
// #TODO: Triangle with zero area

Vertex Triangle::GetVertex(size_t index) const {
  assert(index < 3);
  return vertices_[index];
}

const Eigen::Vector4f& Triangle::GetNormal() const {
  return normal_;
}