#include "vertex.hpp"

Vertex::Vertex(float x, float y, float z)
    : Point(x, y, z), attribute_color_(0, 0, 0) {}

Vertex::Vertex(Eigen::Vector3f vector_3f)
    : Vertex(vector_3f[0], vector_3f[1], vector_3f[2]) {}

Eigen::Vector3f Vertex::GetAttributeColor() const {
  return attribute_color_;
}