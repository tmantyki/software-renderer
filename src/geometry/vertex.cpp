#include "vertex.hpp"
#include "common.hpp"

Vertex::Vertex() : Vertex(0, 0, 0) {}

Vertex::Vertex(float x, float y, float z) : Point(x, y, z) {}

Vertex::Vertex(Vector3 vector) : Point(vector) {}

Vertex::Vertex(Vector4 vector) : Point(vector) {}

Vertex::Vertex(Point& point) : Point(point) {}

// Vector3 Vertex::GetAttributeColor() const {
//   return attribute_color_;
// }