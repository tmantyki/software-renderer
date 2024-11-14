#include <cassert>
#include "line_segment.hpp"

LineSegment::LineSegment(Point a, Point b) : a_(a), b_(b)
{
  assert(a.GetVector() != b.GetVector());
}

Point LineSegment::GetInterpolatedPoint(float t) const
{
  assert(t >= 0 && t <= 1);
  return Point(((1 - t) * a_.GetVector() + t * b_.GetVector())({0, 1, 2}));
}

Point LineSegment::GetPointA() const { return a_; }
Point LineSegment::GetPointB() const { return b_; }