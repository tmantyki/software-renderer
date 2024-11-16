#include "line_segment.hpp"
#include <cassert>

LineSegment::LineSegment(Point a, Point b) : a_(a), b_(b) {
  assert(a.GetVector() != b.GetVector());
}

Point LineSegment::GetInterpolatedPoint(float t) const {
  assert(t >= 0 && t <= 1);
  return Point(((1 - t) * a_.GetVector() + t * b_.GetVector())({0, 1, 2}));
}

float LineSegment::GetPlaneIntersectionParameter(Plane& plane) const {
  // #TODO: handle case when plane is parallel to the line segment
  assert(plane.GetVector().dot(GetDirection().GetVector()) != 0);
  return -(plane.GetVector().dot(GetPointA().GetVector())) /
         (plane.GetVector().dot(GetDirection().GetVector()));
}

Point LineSegment::GetPointA() const {
  return a_;
}
Point LineSegment::GetPointB() const {
  return b_;
}

Direction LineSegment::GetDirection() const {
  return (GetPointB() - GetPointA());
}