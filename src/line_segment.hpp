#ifndef LINE_SEGMENT_HPP
#define LINE_SEGMENT_HPP

#include "point.hpp"
#include "direction.hpp"

class LineSegment
{
public:
  LineSegment() = delete;
  LineSegment(Point a, Point b);
  Point GetInterpolatedPoint(float t) const;
  float GetPlaneIntersectionParameter(Plane &plane) const;
  Point GetPointA() const;
  Point GetPointB() const;
  Direction GetDirection() const;

private:
  Point a_, b_;
};

#endif