#ifndef COORDINATE_HPP
#define COORDINATE_HPP

#include <Eigen/Core>

class Coordinate
{
public:
  Coordinate() = delete;
  Coordinate(float x, float y, float z, float w);
  Coordinate(Eigen::Vector4f vector_4f);
  Eigen::Vector4f GetVector() const;
  bool operator==(const Coordinate &rhs) const;
  bool operator!=(const Coordinate &rhs) const;

protected:
  Eigen::Vector4f vector_;
};

#endif