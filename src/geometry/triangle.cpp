#include <Eigen/Geometry>

#include "common.hpp"
#include "triangle.hpp"

Triangle::Triangle(Vertex vertex_1, Vertex vertex_2, Vertex vertex_3)
    : vertices_{vertex_1, vertex_2, vertex_3},
      normal_((vertex_2 - vertex_1).cross3(vertex_3 - vertex_1)) {}

Triangle::Triangle(Vertex vertex_1,
                   Vertex vertex_2,
                   Vertex vertex_3,
                   Direction normal)
    : vertices_{vertex_1, vertex_2, vertex_3}, normal_(normal.GetVector()) {
  assert((vertex_1 - vertex_2).dot(normal) == 0);
  assert((vertex_1 - vertex_3).dot(normal) == 0);
}
// #TODO: when to handle normals as vectors versus Directions

/* #TODO: Constructing Triangle with zero area
   -> could be dealt by setting normal to null */

Vertex Triangle::GetVertex(size_t index) const {
  assert(index < 3);
  return vertices_[index];
}

const Eigen::Vector4f& Triangle::GetNormal() const {
  return normal_;
}