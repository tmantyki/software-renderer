#include <Eigen/Geometry>

#include "common.hpp"
#include "triangle.hpp"
Triangle::Triangle(Vertex vertex_1, Vertex vertex_2, Vertex vertex_3)
    : vertices_{vertex_1, vertex_2, vertex_3},
      normal_((vertex_2 - vertex_1).cross3(vertex_3 - vertex_1)) {
  assert(normal_[3] == 0);
  assert(fabs((vertex_1 - vertex_2).dot(normal_)) < kFloatTolerance);
  assert(fabs((vertex_1 - vertex_3).dot(normal_)) < kFloatTolerance);
}

Triangle::Triangle(Vertex vertex_1,
                   Vertex vertex_2,
                   Vertex vertex_3,
                   Direction normal)
    : vertices_{vertex_1, vertex_2, vertex_3}, normal_(normal.GetVector()) {
}
// #TODO: when to handle normals as vectors versus Directions

/* #TODO: Constructing Triangle with zero area
   -> could be dealt by setting normal to null */

Vertex Triangle::GetVertex(size_t index) const {
  assert(index < 3);
  return vertices_[index];
}

const Vector4& Triangle::GetNormal() const {
  return normal_;
}