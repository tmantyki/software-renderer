#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "geometry/coordinate.hpp"
#include "geometry/direction.hpp"
#include "geometry/line_segment.hpp"
#include "geometry/plane.hpp"
#include "geometry/point.hpp"
#include "geometry/space.hpp"
#include "geometry/transform.hpp"
#include "geometry/triangle.hpp"
#include "geometry/vertex.hpp"
#include "utility/timer.hpp"

namespace {
TriangleSharedPointer CreateRandomTriangle() {
  Vertex v1(Vector3::Random().eval());
  Vertex v2(Vector3::Random().eval());
  Vertex v3(Vector3::Random().eval());
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
  EXPECT_EQ(Vector4(1.1, 2.2, 3.3, 4.4), c.GetVector());
}

TEST(Coordinate, ConstructorVector) {
  Coordinate c(Vector4(0.1, 0.2, 0.3, 0.4));
  EXPECT_EQ(Vector4(0.1, 0.2, 0.3, 0.4), c.GetVector());
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
  EXPECT_EQ(Vector4(1.1, 2.2, 3.3, 1), p.GetVector());
}

TEST(Point, ConstructorVector) {
  Point p(Vector3(0.1, 0.2, 0.3));
  EXPECT_EQ(Vector4(0.1, 0.2, 0.3, 1), p.GetVector());
}

TEST(Direction, ConstructorFloats) {
  Direction d(1.1, 2.2, 3.3);
  EXPECT_EQ(Vector4(1.1, 2.2, 3.3, 0), d.GetVector());
}

TEST(Direction, ConstructorVector) {
  Direction d(Vector3(0.1, 0.2, 0.3));
  EXPECT_EQ(Vector4(0.1, 0.2, 0.3, 0), d.GetVector());
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
    Point c(Vector3(t * Vector3(x, y, z)));
    EXPECT_EQ(c, ls.GetInterpolatedPoint(t));
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
  EXPECT_EQ(Vector4(1, 2, 3, 4), pl.GetVector());
}

TEST(Plane, ConstructorVector) {
  Plane pl(Vector4(5, 6, 7, 8));
  EXPECT_EQ(Vector4(5, 6, 7, 8), pl.GetVector());
}

TEST(Plane, NormalizedVector) {
  Vector4 vector(0, 2, 0, 3);
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
  EXPECT_EQ(Vector4(1.1, 2.2, 3.3, 1), v.GetVector());
}

TEST(Vertex, ConstructorVector) {
  Vertex v(Vector3(0.1, 0.2, 0.3));
  EXPECT_EQ(Vector4(0.1, 0.2, 0.3, 1), v.GetVector());
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
  Vector4 vector_1 = t.GetVertex(0).GetVector() - t.GetVertex(1).GetVector();
  Vector4 vector_2 = t.GetVertex(1).GetVector() - t.GetVertex(2).GetVector();
  Vector4 vector_3 = t.GetVertex(2).GetVector() - t.GetVertex(0).GetVector();
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

TEST(Space, RemoveMultipleTriangles) {
  Space space;
  std::vector<TriangleSharedPointer> t = ::CreateRandomTriangleVector(8);
  ::EnqueAddMultipleTriangles({0, 1, 2, 3, 4, 5, 6, 7}, t, space);
  space.UpdateSpace();
  space.EnqueueRemoveTriangle(1);
  space.EnqueueRemoveTriangle(2);
  space.EnqueueRemoveTriangle(4);
  space.EnqueueRemoveTriangle(5);
  space.UpdateSpace();
  ::VerifyTriangleCount(4, space);
  ::VerifyTriangleOrder({0, 7, 6, 3}, t, space);
}

TEST(Space, AddAndRemoveEqualAmountOfTriangles) {
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

TEST(Space, AddMoreThanRemoveTriangles) {
  Space space;
  std::vector<TriangleSharedPointer> t = ::CreateRandomTriangleVector(10);
  ::EnqueAddMultipleTriangles({0, 1, 2, 3, 4, 5}, t, space);
  space.UpdateSpace();
  space.EnqueueRemoveTriangle(2);
  space.EnqueueRemoveTriangle(3);
  ::EnqueAddMultipleTriangles({6, 7, 8, 9}, t, space);
  space.UpdateSpace();
  ::VerifyTriangleCount(8, space);
  ::VerifyTriangleOrder({0, 1, 6, 7, 4, 5, 8, 9}, t, space);
}

TEST(Space, RemoveMoreThanAddTriangles) {
  Space space;
  std::vector<TriangleSharedPointer> t = ::CreateRandomTriangleVector(8);
  ::EnqueAddMultipleTriangles({0, 1, 2, 3, 4, 5}, t, space);
  space.UpdateSpace();
  space.EnqueueRemoveTriangle(1);
  space.EnqueueRemoveTriangle(2);
  space.EnqueueRemoveTriangle(3);
  space.EnqueueRemoveTriangle(4);
  ::EnqueAddMultipleTriangles({6, 7}, t, space);
  space.UpdateSpace();
  ::VerifyTriangleCount(4, space);
  ::VerifyTriangleOrder({0, 6, 7, 5}, t, space);
}

TEST(Space, RemoveAllButOneTriangles) {
  Space space;
  std::vector<TriangleSharedPointer> t = ::CreateRandomTriangleVector(8);
  ::EnqueAddMultipleTriangles({0, 1, 2, 3, 4, 5, 6, 7}, t, space);
  space.UpdateSpace();
  space.EnqueueRemoveTriangle(0);
  space.EnqueueRemoveTriangle(1);
  space.EnqueueRemoveTriangle(2);
  space.EnqueueRemoveTriangle(3);
  space.EnqueueRemoveTriangle(4);
  space.EnqueueRemoveTriangle(5);
  space.EnqueueRemoveTriangle(6);
  space.UpdateSpace();
  ::VerifyTriangleCount(1, space);
  ::VerifyTriangleOrder({7}, t, space);
}

TEST(Space, Dehomogenize) {
  Space space;
  TriangleSharedPointer tr = ::CreateRandomTriangle();
  space.EnqueueAddTriangle(tr);
  space.UpdateSpace();
  Matrix4 random_transform = Matrix4::Random();
  space.TransformVertices(random_transform);
  VertexMatrix pre = space.GetVertices();
  space.Dehomogenize();
  VertexMatrix post = space.GetVertices();
  for (int16_t c = 0; c < space.GetVertices().cols(); c++) {
    EXPECT_EQ(pre(0, c) / pre(3, c), post(0, c));
    EXPECT_EQ(pre(1, c) / pre(3, c), post(1, c));
    EXPECT_EQ(pre(2, c) / pre(3, c), post(2, c));
    EXPECT_EQ(1, post(3, c));
  }
}

TEST(Space, TransformVerticesAndNormals) {
  Space space;
  Matrix4 random_transform = Matrix4::Random();
  std::vector<TriangleSharedPointer> t = ::CreateRandomTriangleVector(8);
  ::EnqueAddMultipleTriangles({0, 1, 2, 3, 4, 5, 6, 7}, t, space);
  space.UpdateSpace();
  VertexMatrix pre_vertices = space.GetVertices();
  VertexMatrix pre_normals = space.GetNormals();
  space.TransformVertices(random_transform);
  space.TransformNormals(random_transform);
  VertexMatrix post_vertices = space.GetVertices();
  VertexMatrix post_normals = space.GetNormals();
  EXPECT_EQ(random_transform * pre_vertices, post_vertices);
  EXPECT_EQ(random_transform * pre_normals, post_normals);
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

TEST(Camera, GetAndSetFunctions) {
  Camera cam;
  float pitch = 0.13, yaw = 0.17, roll = 0.23;
  Point new_location(1, 2, 3);
  cam.SetLocation(new_location);
  cam.SetPitch(pitch);
  cam.SetYaw(yaw);
  cam.SetRoll(roll);
  EXPECT_EQ(new_location, cam.GetLocation());
  EXPECT_EQ(pitch, cam.GetPitch());
  EXPECT_EQ(yaw, cam.GetYaw());
  EXPECT_EQ(roll, cam.GetRoll());
}

class CameraTransformTest : public testing::Test {
 protected:
  CameraTransformTest()
      : camera_(camera_transform_.GetCamera()), constructed_camera_(camera_) {}
  void ExpectOutput(Vector4 expectation) {
    camera_transform_.UpdateTransform();
    EXPECT_TRUE(::CamerasAreEqual(constructed_camera_, camera_));
    Vector4 output = camera_transform_.GetMatrix() * input_;
    for (size_t i : {0, 1, 2, 3})
      EXPECT_NEAR(expectation[i], output[i], kFloatTolerance);
  }
  CameraTransform camera_transform_;
  Camera& camera_;
  Camera constructed_camera_;
  Vector4 input_;
};

TEST_F(CameraTransformTest, ConstructorDefault) {
  constructed_camera_ = Camera();
  EXPECT_TRUE(::CamerasAreEqual(constructed_camera_, camera_));
  EXPECT_EQ(Matrix4::Identity(), camera_transform_.GetMatrix());
}

TEST_F(CameraTransformTest, ConstructorWithLocationOffset) {
  float x = 1, y = 2, z = 3;
  constructed_camera_ = {{x, y, z}, 0, 0, 0};
  camera_.SetLocation({x, y, z});
  camera_transform_.UpdateTransform();
  Matrix4 M = Matrix4::Identity();
  M.col(3) = Vector4(-x, -y, -z, 1);
  EXPECT_TRUE(::CamerasAreEqual(camera_, constructed_camera_));
  EXPECT_EQ(M, camera_transform_.GetMatrix());
}

TEST_F(CameraTransformTest, ConstructorWithPitch) {
  float pitch = 1;
  constructed_camera_ = {{0, 0, 0}, pitch, 0, 0};
  camera_.SetPitch(pitch);
  input_ = {0, 0, -1, 1};
  ExpectOutput({0, std::sin(1), -std::cos(1), 1});
}

TEST_F(CameraTransformTest, ConstructorWithYaw) {
  float yaw = 1;
  constructed_camera_ = {{0, 0, 0}, 0, yaw, 0};
  camera_.SetYaw(yaw);
  input_ = {0, 0, -1, 1};
  ExpectOutput({-std::sin(1), 0, -std::cos(1), 1});
}

TEST_F(CameraTransformTest, ConstructorWithRoll) {
  float roll = 1;
  constructed_camera_ = {{0, 0, 0}, 0, 0, roll};
  camera_.SetRoll(roll);
  input_ = {1, 0, 0, 1};
  ExpectOutput({std::cos(1), std::sin(1), 0, 1});
}

TEST_F(CameraTransformTest, ConstructorWithLocationPitchYawRoll) {
  float pitch = std::acos(1 / std::sqrt(2));
  float yaw = std::acos(1 / std::sqrt(3));
  float roll = -kPi * 3 / 4;
  float x_offset = 3, y_offset = -7, z_offset = 13;
  constructed_camera_ = {{x_offset, y_offset, z_offset}, pitch, yaw, roll};
  camera_.SetLocation({x_offset, y_offset, z_offset});
  camera_.SetPitch(pitch);
  camera_.SetYaw(yaw);
  camera_.SetRoll(roll);
  input_ = {1 + x_offset, y_offset, z_offset, 1};
  ExpectOutput({0, -std::sqrt(2.0 / 3), -1 / std::sqrt(3), 1});
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
  Matrix4 M = Matrix4::Zero();
  M(0, 0) = 0.5;
  M(0, 2) = 3;
  M(1, 1) = 0.125;
  M(1, 2) = 3;
  M(2, 2) = -3;
  M(2, 3) = -4;
  M(3, 2) = -1;
  EXPECT_EQ(M, pp.GetMatrix());
}

// #TODO: check normals after clipping

class HomogeneousClippingTest : public testing::Test {
 protected:
  HomogeneousClippingTest()
      : v1_(-2.0, 0.5, 1.5),
        v2_(2.0, 0.5, 1.5),
        v3_(-2.0, -0.5, 1.5),
        tr_(std::make_shared<Triangle>(v1_, v2_, v3_)) {
    space_.EnqueueAddTriangle(tr_);
    space_.UpdateSpace();
  }
  void ExpectTriangle(const Triangle& expected_triangle,
                      const Triangle& actual_triangle) {
    for (size_t i : {0, 1, 2})
      EXPECT_EQ(expected_triangle.GetVertex(i), actual_triangle.GetVertex(i));
  }
  Space space_;
  Vertex v1_;
  Vertex v2_;
  Vertex v3_;
  TriangleSharedPointer tr_;
};

TEST_F(HomogeneousClippingTest, TriangleIsInside) {
  space_.ClipAllTriangles(Axis::kY, AxisDirection::kPositive);
  ::VerifyTriangleCount(1, space_);
  EXPECT_EQ(tr_, space_.GetTriangles()[0]);
}

TEST_F(HomogeneousClippingTest, TriangleIsOutside) {
  space_.ClipAllTriangles(Axis::kZ, AxisDirection::kPositive);
  ::VerifyTriangleCount(0, space_);
}

TEST_F(HomogeneousClippingTest, TriangleIsClippedIntoOne) {
  space_.ClipAllTriangles(Axis::kX, AxisDirection::kNegative);
  ::VerifyTriangleCount(1, space_);
  Triangle tr({2.0, 0.5, 1.5}, {-1.0, -0.25, 1.5}, {-1.0, 0.5, 1.5});
  ExpectTriangle(tr, *space_.GetTriangles()[0]);
}

TEST_F(HomogeneousClippingTest, TriangleIsClippedIntoTwo) {
  space_.ClipAllTriangles(Axis::kX, AxisDirection::kPositive);
  ::VerifyTriangleCount(2, space_);
  Triangle tr_1({1.0, 0.25, 1.5}, {-2.0, -0.5, 1.5}, {1.0, 0.5, 1.5});
  Triangle tr_2({1.0, 0.5, 1.5}, {-2.0, -0.5, 1.5}, {-2.0, 0.5, 1.5});
  ExpectTriangle(tr_1, *space_.GetTriangles()[0]);
  ExpectTriangle(tr_2, *space_.GetTriangles()[1]);
}

TEST(ViewportTransform, ConstructorArguments) {
  int w = 800, h = 600, x_offset = 10, y_offset = -20;
  ViewportTransform vt(w, h, x_offset, y_offset);
  EXPECT_EQ(w, vt.GetWidth());
  EXPECT_EQ(h, vt.GetHeight());
  EXPECT_EQ(x_offset, vt.GetOffsetX());
  EXPECT_EQ(y_offset, vt.GetOffsetY());
}

class ViewportTransformTest : public testing::Test {
 protected:
  ViewportTransformTest()
      : width_(100),
        height_(200),
        x_offset_(300),
        y_offset_(400),
        vt_(width_, height_, x_offset_, y_offset_),
        v_x(3),
        v_y(5),
        v_z(7),
        v_w(1),
        vector_(v_x, v_y, v_z, v_w) {}
  void SetViewportParameters(uint16_t width,
                             uint16_t height,
                             int16_t x_offset,
                             int16_t y_offset) {
    width_ = width;
    height_ = height;
    x_offset_ = x_offset;
    y_offset_ = y_offset;
    vt_ = ViewportTransform(width_, height_, x_offset_, y_offset_);
  }
  uint16_t width_;
  uint16_t height_;
  int16_t x_offset_;
  int16_t y_offset_;
  ViewportTransform vt_;
  float v_x, v_y, v_z, v_w;
  Vector4 vector_;
  Vector4 GetTransformedVector() const { return vt_.GetMatrix() * vector_; }
};

TEST_F(ViewportTransformTest, GetFunctions) {
  EXPECT_EQ(width_, vt_.GetWidth());
  EXPECT_EQ(height_, vt_.GetHeight());
  EXPECT_EQ(x_offset_, vt_.GetOffsetX());
  EXPECT_EQ(y_offset_, vt_.GetOffsetY());
}

TEST_F(ViewportTransformTest, CoordinateZIsUnaffected) {
  SetViewportParameters(13, 17, 19, -23);
  EXPECT_EQ(v_z, GetTransformedVector()[2]);
}

TEST_F(ViewportTransformTest, CoordinateWIsUnaffected) {
  SetViewportParameters(13, 17, 19, -23);
  EXPECT_EQ(v_w, GetTransformedVector()[3]);
}

TEST_F(ViewportTransformTest, TransformAxisX) {
  uint16_t width = 512;
  SetViewportParameters(width, 0, 0, 0);
  EXPECT_EQ((v_x + 1) * width / 2.0, GetTransformedVector()[0]);
}

TEST_F(ViewportTransformTest, TransformAxisY) {
  uint16_t height = 240;
  SetViewportParameters(0, height, 0, 0);
  EXPECT_EQ((1 - v_y) * height / 2.0, GetTransformedVector()[1]);
}

TEST_F(ViewportTransformTest, AddPositiveOffsetX) {
  uint16_t width = 480;
  int16_t x_offset = 100;
  SetViewportParameters(width, 0, x_offset, 0);
  EXPECT_EQ(((v_x + 1) * width / 2.0) + x_offset, GetTransformedVector()[0]);
}

TEST_F(ViewportTransformTest, AddNegativeOffsetX) {
  uint16_t width = 1024;
  int16_t x_offset = -110;
  SetViewportParameters(width, 0, x_offset, 0);
  EXPECT_EQ(((v_x + 1) * width / 2.0) + x_offset, GetTransformedVector()[0]);
}

TEST_F(ViewportTransformTest, AddPositiveOffsetY) {
  uint16_t height = 240;
  int16_t y_offset = 300;
  SetViewportParameters(0, height, 0, y_offset);
  EXPECT_EQ(((1 - v_y) * height / 2.0) + y_offset, GetTransformedVector()[1]);
}

TEST_F(ViewportTransformTest, AddNegativeOffsetY) {
  uint16_t height = 480;
  int16_t y_offset = -150;
  SetViewportParameters(0, height, 0, y_offset);
  EXPECT_EQ(((1 - v_y) * height / 2.0) + y_offset, GetTransformedVector()[1]);
}

TEST_F(ViewportTransformTest, FullTransform) {
  uint16_t width = 800, height = 600;
  int16_t x_offset = 10, y_offset = -20;
  SetViewportParameters(width, height, x_offset, y_offset);
  EXPECT_EQ(((v_x + 1) * width / 2.0) + x_offset, GetTransformedVector()[0]);
  EXPECT_EQ(((1 - v_y) * height / 2.0) + y_offset, GetTransformedVector()[1]);
  EXPECT_EQ(v_z, GetTransformedVector()[2]);
  EXPECT_EQ(v_w, GetTransformedVector()[3]);
}

TEST(TransformPipeline, ConstructorObjects) {
  std::shared_ptr<CameraTransform> camera = std::make_shared<CameraTransform>();
  std::shared_ptr<PerspectiveProjection> perspective =
      std::make_shared<PerspectiveProjection>(1, 100, -50, 50, 40, -40);
  std::shared_ptr<ViewportTransform> viewport =
      std::make_shared<ViewportTransform>(800, 600, 0, 0);
  TransformPipeline pipeline(camera, perspective, viewport);
  EXPECT_EQ(camera, pipeline.GetCameraTransform());
  EXPECT_EQ(perspective, pipeline.GetPerspectiveProjection());
  EXPECT_EQ(viewport, pipeline.GetViewportTransform());
}

class TransformPipelineTest : public testing ::Test {
 protected:
  TransformPipelineTest()
      : v1_(2, 0, 0),
        v2_(4, 0, 0),
        v3_(4, 0, 2),
        camera_(std::make_shared<CameraTransform>(
            Camera({0, 0, 0}, kPi / 2, -kPi / 2, -kPi / 2))),
        perspective_(
            std::make_shared<PerspectiveProjection>(1, 10, -1, 1, 1, -1)),
        viewport_(std::make_shared<ViewportTransform>(800, 800, 0, 0)),
        pipeline_(camera_, perspective_, viewport_) {
    world_space_.EnqueueAddTriangle(std::make_shared<Triangle>(v1_, v2_, v3_));
    world_space_.UpdateSpace();
  }
  void SetAndUpdateCameraLocation(const Point& location) {
    camera_->GetCamera().SetLocation(location);
    camera_->UpdateTransform();
    pipeline_.RunPipeline(world_space_);
  }
  void ExpectVertices(const Triangle& expected_triangle,
                      size_t triangle_index) {
    size_t i = triangle_index;
    for (size_t k = 0; k < kVerticesPerTriangle; k++)
      for (size_t d : {0, 1, 2, 3})
        EXPECT_NEAR(expected_triangle.GetVertex(k).GetVector()[d],
                    pipeline_.GetOutputSpace().GetVertices().col(
                        i * kVerticesPerTriangle + k)[d],
                    kFloatTolerance);
  }
  Space world_space_;
  Vertex v1_, v2_, v3_;
  std::shared_ptr<CameraTransform> camera_;
  std::shared_ptr<PerspectiveProjection> perspective_;
  std::shared_ptr<ViewportTransform> viewport_;
  TransformPipeline pipeline_;
  float expected_z_ = (20 * (1 - (1 / 2.0)) / 9) - 1;
};

TEST_F(TransformPipelineTest, SingleFullyVisibleTriangle) {
  SetAndUpdateCameraLocation({3, 2, 1});
  EXPECT_EQ(1, pipeline_.GetOutputSpace().GetTriangleCount());
  Triangle tr({600, 600, expected_z_}, {200, 600, expected_z_},
              {200, 200, expected_z_});
  ExpectVertices(tr, 0);
}

TEST_F(TransformPipelineTest, SingleFullyClippedTriangle) {
  SetAndUpdateCameraLocation({-3, 2, 1});
  EXPECT_EQ(0, pipeline_.GetOutputSpace().GetTriangleCount());
}

TEST_F(TransformPipelineTest, SingleTriangleClippedIntoOne) {
  SetAndUpdateCameraLocation({3, 2, 3});
  EXPECT_EQ(1, pipeline_.GetOutputSpace().GetTriangleCount());
  Triangle tr({200, 600, expected_z_}, {400, 800, expected_z_},
              {200, 800, expected_z_});
  ExpectVertices(tr, 0);
}

TEST_F(TransformPipelineTest, SingleTriangleClippedIntoTwo) {
  SetAndUpdateCameraLocation({3, 2, -1});
  EXPECT_EQ(2, pipeline_.GetOutputSpace().GetTriangleCount());
  Triangle tr_1({400, 0, expected_z_}, {600, 200, expected_z_},
                {200, 0, expected_z_});
  Triangle tr_2({200, 0, expected_z_}, {600, 200, expected_z_},
                {200, 200, expected_z_});
  ExpectVertices(tr_1, 0);
  ExpectVertices(tr_2, 1);
}
