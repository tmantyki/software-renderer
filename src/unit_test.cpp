#include <gtest/gtest.h>

#include "coordinate.hpp"
#include "point.hpp"
#include "direction.hpp"
#include "line_segment.hpp"
#include "plane.hpp"

TEST(Coordinate, ConstructorDefault)
{
  Coordinate c;
  EXPECT_EQ(Eigen::Vector4f(0, 0, 0, 0), c.GetVector());
}

TEST(Coordinate, ConstructorFloats)
{
  Coordinate c(1.1, 2.2, 3.3, 4.4);
  EXPECT_EQ(Eigen::Vector4f(1.1, 2.2, 3.3, 4.4), c.GetVector());
}

TEST(Coordinate, ConstructorVector)
{
  Coordinate c(Eigen::Vector4f(0.1, 0.2, 0.3, 0.4));
  EXPECT_EQ(Eigen::Vector4f(0.1, 0.2, 0.3, 0.4), c.GetVector());
}

TEST(Coordinate, EqualityBetweenDerivedObjects)
{
  Coordinate a(0, 0, 0, 0), b(0, 0, 0, 1), c(0.0, 0.0, 0.0, 0.0);
  Point p(0, 0, 0);
  Direction d(0, 0, 0);
  EXPECT_TRUE(a != b);
  EXPECT_TRUE(a == c);
  EXPECT_TRUE(a != p);
  EXPECT_TRUE(b == p);
  EXPECT_TRUE(a == d);
  EXPECT_TRUE(b != d);
}

TEST(Point, ConstructorDefault)
{
  Point p;
  EXPECT_EQ(Eigen::Vector4f(0, 0, 0, 1), p.GetVector());
}

TEST(Point, ConstructorFloats)
{
  Point p(1.1, 2.2, 3.3);
  EXPECT_EQ(Eigen::Vector4f(1.1, 2.2, 3.3, 1), p.GetVector());
}

TEST(Point, ConstructorVector)
{
  Point p(Eigen::Vector3f(0.1, 0.2, 0.3));
  EXPECT_EQ(Eigen::Vector4f(0.1, 0.2, 0.3, 1), p.GetVector());
}

TEST(Direction, ConstructorDefault)
{
  Direction d;
  EXPECT_EQ(Eigen::Vector4f(0, 0, 0, 0), d.GetVector());
}

TEST(Direction, ConstructorFloats)
{
  Direction d(1.1, 2.2, 3.3);
  EXPECT_EQ(Eigen::Vector4f(1.1, 2.2, 3.3, 0), d.GetVector());
}

TEST(Direction, ConstructorVector)
{
  Direction d(Eigen::Vector3f(0.1, 0.2, 0.3));
  EXPECT_EQ(Eigen::Vector4f(0.1, 0.2, 0.3, 0), d.GetVector());
}

TEST(LineSegment, ConstructorPoints)
{
  Point a(0, 0, 0), b(1, 2, 3);
  LineSegment ls(a, b);
  EXPECT_EQ(a, ls.GetPointA());
  EXPECT_EQ(b, ls.GetPointB());
}

TEST(LineSegment, InterpolatePoints)
{
  float t, x = -12, y = 3, z = 170;
  Point a(0, 0, 0), b(x, y, z);
  LineSegment ls(a, b);
  for (t = 0; t <= 1; t += 0.0137)
  {
    Point c(t * Eigen::Vector3f(x, y, z));
    EXPECT_EQ(Point(t * Eigen::Vector3f(x, y, z)), ls.GetInterpolatedPoint(t));
  }
}

TEST(Plane, ConstructorFloats)
{
  Plane pl(1, 2, 3, 4);
  EXPECT_EQ(Eigen::Vector4f(1, 2, 3, 4), pl.GetVector());
}

TEST(Plane, ConstructorVector)
{
  Plane pl(Eigen::Vector4f(5, 6, 7, 8));
  EXPECT_EQ(Eigen::Vector4f(5, 6, 7, 8), pl.GetVector());
}

TEST(Point, PointDistanceFromPlane)
{
  Plane pl(0, 1, 0, 0.5);
  Point p(2, 13, 66);
  EXPECT_EQ(13, p.DistanceFromPlane(pl));
  std::cout << "Distance from origin: " << pl.DistanceFromOrigin() << "\n";
}