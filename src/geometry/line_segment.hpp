#pragma once

#include "direction.hpp"
#include "point.hpp"

class LineSegment {
 public:
  LineSegment() = delete;
  LineSegment(Point a, Point b) : a_(a), b_(b) {
    assert(a.GetVector() != b.GetVector());
  }
  Point GetInterpolatedPoint(float t) const {
    assert(t >= 0 && t <= 1);
    return Point(Vector4(((1 - t) * a_.GetVector() + t * b_.GetVector())));
  }
  float GetPlaneIntersectionParameter(const Plane& plane) const {
    // #TODO: handle case when plane is parallel to the line segment
    assert(plane.GetVector().dot(GetDirection().GetVector()) != 0);
    return -(plane.GetVector().dot(GetPointA().GetVector())) /
           (plane.GetVector().dot(GetDirection().GetVector()));
  }
  Point GetPointA() const { return a_; }
  Point GetPointB() const { return b_; }
  Direction GetDirection() const { return (GetPointB() - GetPointA()); }

 private:
  Point a_, b_;
};
