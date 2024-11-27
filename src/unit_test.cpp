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
#include "transform.hpp"
#include "triangle.hpp"
#include "vertex.hpp"

namespace {
TriangleSharedPointer CreateRandomTriangle() {
  Vertex v1(Eigen::Vector3f::Random());
  Vertex v2(Eigen::Vector3f::Random());
  Vertex v3(Eigen::Vector3f::Random());
  return std::make_shared<Triangle>(v1, v2, v3);
}

std::vector<TriangleSharedPointer> CreateRandomTriangleVector(size_t length) {
  std::vector<TriangleSharedPointer> triangle_array;
  for (size_t i = 0; i < length; i++) {
    triangle_array.push_back(CreateRandomTriangle());
  }
  return triangle_array;
}

void EnqueAddMultipleTriangles(
    std::vector<size_t> ordered_triangle_indices,
    std::vector<TriangleSharedPointer>& triangle_ptr_vector,
    Space& space) {
  for (size_t i : ordered_triangle_indices) {
    space.EnqueueAddTriangle(triangle_ptr_vector.at(i));
  }
}

void VerifyTriangleCount(size_t vector_count, Space& space) {
  EXPECT_EQ(kDimensions, space.GetVertices().rows());
  EXPECT_EQ(3 * vector_count, space.GetVertices().cols());
  EXPECT_EQ(vector_count, space.GetNormals().cols());
  EXPECT_EQ(vector_count, space.GetTriangleCount());
}

void VerifyTriangleOrder(
    std::vector<size_t> ordered_indices,
    std::vector<TriangleSharedPointer>& triangle_ptr_vector,
    Space& space) {
  size_t n = 0;
  for (size_t i : ordered_indices) {
    for (size_t k : {0, 1, 2}) {
      EXPECT_EQ(triangle_ptr_vector[i]->GetVertex(k).GetVector(),
                space.GetVertices().col(3 * n + k));
    }
    EXPECT_EQ(triangle_ptr_vector[i]->GetNormal(), space.GetNormals().col(n));
    n++;
  }
}

bool CamerasAreEqual(Camera& lhs, Camera& rhs) {
  if (lhs.GetLocation() != rhs.GetLocation())
    return false;
  if (lhs.GetPitch() != rhs.GetPitch())
    return false;
  if (lhs.GetYaw() != rhs.GetYaw())
    return false;
  if (lhs.GetRoll() != rhs.GetRoll())
    return false;
  return true;
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

TEST(Triangle, Normal) {
  Vertex v1{1, 1, 1}, v2{2, 3, 4}, v3{7, 6, 5};
  Triangle t(v1, v2, v3);
  Eigen::Vector4f vector_1 =
      t.GetVertex(0).GetVector() - t.GetVertex(1).GetVector();
  Eigen::Vector4f vector_2 =
      t.GetVertex(1).GetVector() - t.GetVertex(2).GetVector();
  Eigen::Vector4f vector_3 =
      t.GetVertex(2).GetVector() - t.GetVertex(0).GetVector();
  EXPECT_EQ(0, vector_1.dot(t.GetNormal()));
  EXPECT_EQ(0, vector_2.dot(t.GetNormal()));
  EXPECT_EQ(0, vector_3.dot(t.GetNormal()));
}

TEST(Space, AddSingleTriangle) {
  Space space;
  TriangleSharedPointer t_ptr = ::CreateRandomTriangle();
  ::VerifyTriangleCount(0, space);
  EXPECT_EQ(nullptr, space.GetTriangles()[0]);
  space.EnqueueAddTriangle(t_ptr);
  ::VerifyTriangleCount(0, space);
  EXPECT_EQ(nullptr, space.GetTriangles()[0]);
  space.UpdateSpace();
  ::VerifyTriangleCount(1, space);
  EXPECT_EQ(t_ptr, space.GetTriangles()[0]);
}

TEST(Space, AddMultipleTriangles) {
  Space space;
  std::vector<TriangleSharedPointer> t = ::CreateRandomTriangleVector(4);
  ::EnqueAddMultipleTriangles({0, 1, 2, 3}, t, space);
  space.UpdateSpace();
  ::VerifyTriangleCount(4, space);
  ::VerifyTriangleOrder({0, 1, 2, 3}, t, space);
}

TEST(Space, RemoveSingleTriangleFromTop) {
  Space space;
  std::vector<TriangleSharedPointer> t = ::CreateRandomTriangleVector(3);
  ::EnqueAddMultipleTriangles({0, 1, 2}, t, space);
  space.UpdateSpace();
  space.EnqueueRemoveTriangle(0);
  space.UpdateSpace();
  ::VerifyTriangleCount(2, space);
  ::VerifyTriangleOrder({2, 1}, t, space);
}

TEST(Space, RemoveSingleTriangleFromMiddle) {
  Space space;
  std::vector<TriangleSharedPointer> t = ::CreateRandomTriangleVector(3);
  ::EnqueAddMultipleTriangles({0, 1, 2}, t, space);
  space.UpdateSpace();
  space.EnqueueRemoveTriangle(1);
  space.UpdateSpace();
  ::VerifyTriangleCount(2, space);
  ::VerifyTriangleOrder({0, 2}, t, space);
}

TEST(Space, RemoveSingleTriangleFromBottom) {
  Space space;
  std::vector<TriangleSharedPointer> t = ::CreateRandomTriangleVector(3);
  ::EnqueAddMultipleTriangles({0, 1, 2}, t, space);
  space.UpdateSpace();
  space.EnqueueRemoveTriangle(2);
  space.UpdateSpace();
  ::VerifyTriangleCount(2, space);
  ::VerifyTriangleOrder({0, 1}, t, space);
}

TEST(Space, AddAndRemoveMultipleTriangles) {
  Space space;
  std::vector<TriangleSharedPointer> t = ::CreateRandomTriangleVector(6);
  ::EnqueAddMultipleTriangles({0, 1, 2}, t, space);
  space.UpdateSpace();
  space.EnqueueRemoveTriangle(0);
  space.EnqueueRemoveTriangle(1);
  ::EnqueAddMultipleTriangles({3, 4, 5}, t, space);
  space.UpdateSpace();
  ::VerifyTriangleCount(4, space);
  ::VerifyTriangleOrder({3, 4, 2, 5}, t, space);
}

TEST(Camera, ConstructorDefault) {
  Camera cam;
  EXPECT_EQ(Point(0, 0, 0), cam.GetLocation());
  EXPECT_FLOAT_EQ(0, cam.GetPitch());
  EXPECT_FLOAT_EQ(0, cam.GetYaw());
  EXPECT_FLOAT_EQ(0, cam.GetRoll());
}

TEST(Camera, ConstructorArguments) {
  Camera cam_1({1, 2, 3}, 1.1, 2.2, 3.3);
  EXPECT_EQ(Point(1, 2, 3), cam_1.GetLocation());
  EXPECT_FLOAT_EQ(1.1, cam_1.GetPitch());
  EXPECT_FLOAT_EQ(2.2, cam_1.GetYaw());
  EXPECT_FLOAT_EQ(3.3, cam_1.GetRoll());
  Camera cam_2({-1, -2, -3});
  EXPECT_EQ(Point(-1, -2, -3), cam_2.GetLocation());
  EXPECT_FLOAT_EQ(0, cam_2.GetPitch());
  EXPECT_FLOAT_EQ(0, cam_2.GetYaw());
  EXPECT_FLOAT_EQ(0, cam_2.GetRoll());
}

TEST(Transform, ConstructorMatrix) {
  Eigen::Matrix4f M = Eigen::Matrix4f::Random();
  Transform t(M);
  EXPECT_EQ(M, t.GetMatrix());
}

TEST(CameraTransform, ConstructorDefault) {
  Camera c;
  CameraTransform ct;
  EXPECT_TRUE(::CamerasAreEqual(c, ct.GetCamera()));
  EXPECT_EQ(Eigen::Matrix4f::Identity(), ct.GetMatrix());
}

TEST(CameraTransform, ConstructorWithLocationOffset) {
  Camera c({1, 2, 3});
  CameraTransform ct(c);
  EXPECT_TRUE(::CamerasAreEqual(c, ct.GetCamera()));
  Eigen::Matrix4f M = Eigen::Matrix4f::Identity();
  M.col(3) = Eigen::Vector4f(-1, -2, -3, 1);
  EXPECT_EQ(M, ct.GetMatrix());
}

// #TODO: check signs and direction conventions
TEST(CameraTransform, ConstructorWithPitch) {
  Camera c({0, 0, 0}, 1, 0, 0);
  CameraTransform ct(c);
  Eigen::Vector4f p(0, 0, -1, 1);
  Eigen::Vector4f p_transformed = ct.GetMatrix() * p;
  EXPECT_FLOAT_EQ(0, p_transformed[0]);
  EXPECT_FLOAT_EQ(std::sin(1), p_transformed[1]);
  EXPECT_FLOAT_EQ(-std::cos(1), p_transformed[2]);
  EXPECT_FLOAT_EQ(1, p_transformed[3]);
}

TEST(CameraTransform, ConstructorWithYaw) {
  Camera c({0, 0, 0}, 0, 1, 0);
  CameraTransform ct(c);
  Eigen::Vector4f p(0, 0, -1, 1);
  Eigen::Vector4f p_transformed = ct.GetMatrix() * p;
  EXPECT_FLOAT_EQ(-std::sin(1), p_transformed[0]);
  EXPECT_FLOAT_EQ(0, p_transformed[1]);
  EXPECT_FLOAT_EQ(-std::cos(1), p_transformed[2]);
  EXPECT_FLOAT_EQ(1, p_transformed[3]);
}

TEST(CameraTransform, ConstructorWithRoll) {
  Camera c({0, 0, 0}, 0, 0, 1);
  CameraTransform ct(c);
  Eigen::Vector4f p(1, 0, 0, 1);
  Eigen::Vector4f p_transformed = ct.GetMatrix() * p;
  EXPECT_FLOAT_EQ(std::cos(1), p_transformed[0]);
  EXPECT_FLOAT_EQ(std::sin(1), p_transformed[1]);
  EXPECT_FLOAT_EQ(0, p_transformed[2]);
  EXPECT_FLOAT_EQ(1, p_transformed[3]);
}

// #TODO: more thorough precision measurements
TEST(CameraTransform, ConstructorWithPitchYawRoll) {
  float abs_error = 0.000001;
  Camera c({0, 0, 0}, std::acos(1 / std::sqrt(2)), std::acos(1 / std::sqrt(3)),
           -kPi * 3 / 4);
  CameraTransform ct(c);
  Eigen::Vector4f p(1, 0, 0, 1);
  Eigen::Vector4f p_transformed = ct.GetMatrix() * p;
  EXPECT_NEAR(0, p_transformed[0], abs_error);
  EXPECT_NEAR(-std::sqrt(2.0 / 3), p_transformed[1], abs_error);
  EXPECT_NEAR(-1 / std::sqrt(3), p_transformed[2], abs_error);
  EXPECT_NEAR(1, p_transformed[3], abs_error);
}

TEST(PespectiveProjection, ConstructorArguments) {
  float near = 1, far = 2, left = 4, right = 8, top = 32, bottom = 16;
  PerspectiveProjection pp(near, far, left, right, top, bottom);
  EXPECT_EQ(near, pp.GetNear());
  EXPECT_EQ(far, pp.GetFar());
  EXPECT_EQ(left, pp.GetLeft());
  EXPECT_EQ(right, pp.GetRight());
  EXPECT_EQ(top, pp.GetTop());
  EXPECT_EQ(bottom, pp.GetBottom());
  Eigen::Matrix4f M = Eigen::Matrix4f::Zero();
  M(0, 0) = 0.5;
  M(0, 2) = 3;
  M(1, 1) = 0.125;
  M(1, 2) = 3;
  M(2, 2) = -3;
  M(2, 3) = -4;
  M(3, 2) = -1;
  EXPECT_EQ(M, pp.GetMatrix());
}

// TEST(SpaceTest, TriangleClipping) {
//   Space space;
//   std::vector<TriangleSharedPointer> t = ::CreateRandomTriangleVector(8);
//   ::EnqueAddMultipleTriangles({0, 1, 2, 3, 4, 5, 6, 7}, t, space);
//   space.UpdateSpace();

//   Plane pl(1, 0, 0, 0);
//   space.ClipTriangles(pl);
// }