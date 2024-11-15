#ifndef VERTEX_HPP
#define VERTEX_HPP

#include "point.hpp"

class Vertex : public Point
{
public:
  Vertex() = delete;
  Vertex(float x, float y, float z);
  Vertex(Eigen::Vector3f vector_3f);
  Eigen::Vector3f GetAttributeColor() const;

private:
  Eigen::Vector3f attribute_color_;
};

#endif