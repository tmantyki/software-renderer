#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "coordinate.hpp"
#include "direction.hpp"
#include "line_segment.hpp"
#include "plane.hpp"
#include "point.hpp"
#include "space.hpp"
#include "timer.hpp"
#include "triangle.hpp"
#include "vertex.hpp"

namespace {
Triangle CreateRandomTriangle() {
  Vertex v1(Eigen::Vector3f::Random());
  Vertex v2(Eigen::Vector3f::Random());
  Vertex v3(Eigen::Vector3f::Random());
  Triangle ret(v1, v2, v3);
  return Triangle(v1, v2, v3);
}
}  // namespace

// Replace delays with busy loops
TEST(Timer, BasicOperation) {
  Timer t("UNIT_TEST");
  t.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  t.Pause();
  EXPECT_LE(1.0, t.GetDuration());
  EXPECT_GE(2.0, t.GetDuration());
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  EXPECT_LE(1.0, t.GetDuration());
  EXPECT_GE(2.0, t.GetDuration());
  t.Continue();
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  t.Stop(false);
  EXPECT_LE(2.0, t.GetDuration());
  EXPECT_GE(3.0, t.GetDuration());
  t.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  t.Stop(false);
  EXPECT_LE(1.0, t.GetDuration());
  EXPECT_GE(2.0, t.GetDuration());
}

TEST(Coordinate, ConstructorFloats) {
  Coordinate c(1.1, 2.2, 3.3, 4.4);
  EXPECT_EQ(Eigen::Vector4f(1.1, 2.2, 3.3, 4.4), c.GetVector());
}

TEST(Coordinate, ConstructorVector) {
  Coordinate c(Eigen::Vector4f(0.1, 0.2, 0.3, 0.4));
  EXPECT_EQ(Eigen::Vector4f(0.1, 0.2, 0.3, 0.4), c.GetVector());
}

TEST(Coordinate, EqualityBetweenDerivedObjects) {
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

TEST(Point, ConstructorFloats) {
  Point p(1.1, 2.2, 3.3);
  EXPECT_EQ(Eigen::Vector4f(1.1, 2.2, 3.3, 1), p.GetVector());
}

TEST(Point, ConstructorVector) {
  Point p(Eigen::Vector3f(0.1, 0.2, 0.3));
  EXPECT_EQ(Eigen::Vector4f(0.1, 0.2, 0.3, 1), p.GetVector());
}

TEST(Direction, ConstructorFloats) {
  Direction d(1.1, 2.2, 3.3);
  EXPECT_EQ(Eigen::Vector4f(1.1, 2.2, 3.3, 0), d.GetVector());
}

TEST(Direction, ConstructorVector) {
  Direction d(Eigen::Vector3f(0.1, 0.2, 0.3));
  EXPECT_EQ(Eigen::Vector4f(0.1, 0.2, 0.3, 0), d.GetVector());
}

TEST(LineSegment, ConstructorPoints) {
  Point a(0, 0, 0), b(1, 2, 3);
  LineSegment ls(a, b);
  EXPECT_EQ(a, ls.GetPointA());
  EXPECT_EQ(b, ls.GetPointB());
}

TEST(LineSegment, GetInterpolatedPoint) {
  float t, x = -12, y = 3, z = 170;
  Point a(0, 0, 0), b(x, y, z);
  LineSegment ls(a, b);
  for (t = 0; t <= 1; t += 0.0137) {
    Point c(t * Eigen::Vector3f(x, y, z));
    EXPECT_EQ(Point(t * Eigen::Vector3f(x, y, z)), ls.GetInterpolatedPoint(t));
  }
}

TEST(LineSegment, GetPlaneIntersectionParameter) {
  Plane pl(1, 0, 0, 0);
  LineSegment ls({-2, 0, -10}, {1, 4.5, -30});
  EXPECT_FLOAT_EQ(2.0 / 3, ls.GetPlaneIntersectionParameter(pl));
}

TEST(LineSegment, GetDirection) {
  LineSegment ls({0, 1, 2}, {5, 4, 6});
  EXPECT_EQ(Direction(5, 3, 4), ls.GetDirection());
}

TEST(Plane, ConstructorFloats) {
  Plane pl(1, 2, 3, 4);
  EXPECT_EQ(Eigen::Vector4f(1, 2, 3, 4), pl.GetVector());
}

TEST(Plane, ConstructorVector) {
  Plane pl(Eigen::Vector4f(5, 6, 7, 8));
  EXPECT_EQ(Eigen::Vector4f(5, 6, 7, 8), pl.GetVector());
}

TEST(Plane, NormalizedVector) {
  Eigen::Vector4f vector(0, 2, 0, 3);
  Plane pl(vector);
  EXPECT_EQ(vector / 2, pl.GetVectorNormalized());
}

TEST(Plane, SignedDistanceFromOrigin) {
  Plane pl(-10, 0, 0, 2);
  EXPECT_FLOAT_EQ(0.2, pl.SignedDistanceFromOrigin());
}

TEST(Point, PointSignedDistanceFromPlane) {
  Plane pl(0, 2, 0, 2);
  Point p(2, 13, 66);
  EXPECT_FLOAT_EQ(14, p.SignedDistanceFromPlane(pl));
  pl = {0, -3, 0, 3};
  p = {2, 13, 66};
  EXPECT_FLOAT_EQ(-12, p.SignedDistanceFromPlane(pl));
  pl = {0, -5, 0, -5};
  p = {2, 13, 66};
  EXPECT_FLOAT_EQ(-14, p.SignedDistanceFromPlane(pl));
}

TEST(Point, PointMinusPoint) {
  Point p1(8, 7, 6), p2(1, 2, 3);
  EXPECT_EQ(Direction(7, 5, 3), p1 - p2);
}

TEST(Vertex, ConstructorFloats) {
  Vertex v(1.1, 2.2, 3.3);
  EXPECT_EQ(Eigen::Vector4f(1.1, 2.2, 3.3, 1), v.GetVector());
  EXPECT_EQ(Eigen::Vector3f(0, 0, 0), v.GetAttributeColor());
}

TEST(Vertex, ConstructorVector) {
  Vertex v(Eigen::Vector3f(0.1, 0.2, 0.3));
  EXPECT_EQ(Eigen::Vector4f(0.1, 0.2, 0.3, 1), v.GetVector());
  EXPECT_EQ(Eigen::Vector3f(0, 0, 0), v.GetAttributeColor());
}

TEST(Triangle, ConstructorVertices) {
  Vertex v1{1, 1, 1}, v2{2, 3, 4}, v3{7, 6, 5};
  Triangle t(v1, v2, v3);
  EXPECT_EQ(t.GetVertex(0), v1);
  EXPECT_EQ(t.GetVertex(1), v2);
  EXPECT_EQ(t.GetVertex(2), v3);
}
// #TODO: test normals for Triangle

TEST(Space, AddSingleTriangle) {
  Space space;
  Triangle t1 = ::CreateRandomTriangle();
  EXPECT_EQ(4, space.GetVertices().rows());
  EXPECT_EQ(0, space.GetVertices().cols());
  EXPECT_EQ(0, space.GetTriangleCount());
  EXPECT_EQ(nullptr, space.GetTriangles()[0]);
  space.EnqueueAddTriangle(t1);
  EXPECT_EQ(4, space.GetVertices().rows());
  EXPECT_EQ(0, space.GetVertices().cols());
  EXPECT_EQ(0, space.GetTriangleCount());
  EXPECT_EQ(nullptr, space.GetTriangles()[0]);
  space.UpdateSpace();
  EXPECT_EQ(4, space.GetVertices().rows());
  EXPECT_EQ(3, space.GetVertices().cols());
  EXPECT_EQ(1, space.GetTriangleCount());
  EXPECT_EQ(&t1, space.GetTriangles()[0]);
}

TEST(Space, AddMultipleTriangles) {
  Space space;
  Triangle t1 = ::CreateRandomTriangle();
  Triangle t2 = ::CreateRandomTriangle();
  Triangle t3 = ::CreateRandomTriangle();
  Triangle t4 = ::CreateRandomTriangle();
  space.EnqueueAddTriangle(t1);
  space.EnqueueAddTriangle(t2);
  space.EnqueueAddTriangle(t3);
  space.EnqueueAddTriangle(t4);
  space.UpdateSpace();
  EXPECT_EQ(4, space.GetVertices().rows());
  EXPECT_EQ(12, space.GetVertices().cols());
  EXPECT_EQ(4, space.GetTriangleCount());
  EXPECT_EQ(&t1, space.GetTriangles()[0]);
  EXPECT_EQ(&t2, space.GetTriangles()[1]);
  EXPECT_EQ(&t3, space.GetTriangles()[2]);
  EXPECT_EQ(&t4, space.GetTriangles()[3]);
}

TEST(Space, RemoveSingleTriangleFromTop) {
  Space space;
  Triangle t[3] = {::CreateRandomTriangle(), ::CreateRandomTriangle(),
                   ::CreateRandomTriangle()};
  space.EnqueueAddTriangle(t[0]);
  space.EnqueueAddTriangle(t[1]);
  space.EnqueueAddTriangle(t[2]);
  space.UpdateSpace();
  space.EnqueueRemoveTriangle(0);
  space.UpdateSpace();
  EXPECT_EQ(4, space.GetVertices().rows());
  EXPECT_EQ(6, space.GetVertices().cols());
  EXPECT_EQ(2, space.GetTriangleCount());
  Eigen::Matrix<float, 4, Eigen::Dynamic> M;
  size_t n = 0;
  for (size_t i : {2, 1})  // Expected reordering
  {
    for (size_t k : {0, 1, 2}) {
      EXPECT_EQ(t[i].GetVertex(k).GetVector(),
                space.GetVertices().col(3 * n + k));
    }
    n++;
  }
}

TEST(Space, RemoveSingleTriangleFromMiddle) {
  Space space;
  Triangle t[3] = {::CreateRandomTriangle(), ::CreateRandomTriangle(),
                   ::CreateRandomTriangle()};
  space.EnqueueAddTriangle(t[0]);
  space.EnqueueAddTriangle(t[1]);
  space.EnqueueAddTriangle(t[2]);
  space.UpdateSpace();
  space.EnqueueRemoveTriangle(1);
  space.UpdateSpace();
  EXPECT_EQ(4, space.GetVertices().rows());
  EXPECT_EQ(6, space.GetVertices().cols());
  EXPECT_EQ(2, space.GetTriangleCount());
  Eigen::Matrix<float, 4, Eigen::Dynamic> M;
  size_t n = 0;
  for (size_t i : {0, 2})  // Expected reordering
  {
    for (size_t k : {0, 1, 2}) {
      EXPECT_EQ(t[i].GetVertex(k).GetVector(),
                space.GetVertices().col(3 * n + k));
    }
    n++;
  }
}

TEST(Space, RemoveSingleTriangleFromBottom) {
  Space space;
  Triangle t[3] = {::CreateRandomTriangle(), ::CreateRandomTriangle(),
                   ::CreateRandomTriangle()};
  space.EnqueueAddTriangle(t[0]);
  space.EnqueueAddTriangle(t[1]);
  space.EnqueueAddTriangle(t[2]);
  space.UpdateSpace();
  space.EnqueueRemoveTriangle(2);
  space.UpdateSpace();
  EXPECT_EQ(4, space.GetVertices().rows());
  EXPECT_EQ(6, space.GetVertices().cols());
  EXPECT_EQ(2, space.GetTriangleCount());
  Eigen::Matrix<float, 4, Eigen::Dynamic> M;
  size_t n = 0;
  for (size_t i : {0, 1})  // Expected reordering
  {
    for (size_t k : {0, 1, 2}) {
      EXPECT_EQ(t[i].GetVertex(k).GetVector(),
                space.GetVertices().col(3 * n + k));
    }
    n++;
  }
}