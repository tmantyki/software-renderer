#ifndef LINE_SEGMENT_HPP
#define LINE_SEGMENT_HPP

#include "point.hpp"

class LineSegment
{
public:
  LineSegment(Point a, Point b);
  Point GetInterpolatedPoint(float t) const;
  Point GetPointA() const;
  Point GetPointB() const;
private:
  Point a_, b_;
};

#endif