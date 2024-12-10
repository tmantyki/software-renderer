#ifndef COMMON_HPP
#define COMMON_HPP

#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>

#include <Eigen/Core>
#include <Eigen/Geometry>

constexpr float kPi = 3.14159265358979323846264338;
constexpr float kFloatTolerance = 1.0E-6;
constexpr size_t kDimensions = 4;
constexpr size_t kVerticesPerTriangle = 3;
constexpr size_t kViewportDimensions = 3;
constexpr size_t kMaxTriangles = 128;
constexpr size_t kNumberOfClippingPlanes = 6;

class Point;
class Space;
class Triangle;

typedef std::array<Point, 2> TrianglePlaneIntersections;
typedef std::shared_ptr<Space> SpaceSharedPointer;
typedef std::shared_ptr<Triangle> TriangleSharedPointer;
typedef Eigen::Vector3f Vector3;
typedef Eigen::Vector4f Vector4;
typedef Eigen::Matrix4f Matrix4;
typedef Eigen::Quaternionf Quaternion;
typedef Eigen::Array<int, kVerticesPerTriangle, Eigen::Dynamic> ClippingMask;
typedef Eigen::
    Matrix<float, kDimensions, Eigen::Dynamic, 0, kDimensions, kMaxTriangles>
        NormalMatrix;
typedef Eigen::Matrix<float,
                      kDimensions,
                      Eigen::Dynamic,
                      0,
                      kDimensions,
                      kVerticesPerTriangle * kMaxTriangles>
    VertexMatrix;

enum class TriangleClipMode { kIncludeReference, kExcludeReference };
enum TriangleEdge { kAB = 0, kAC = 1, kBC = 2 };
enum Axis { kX = 0, kY = 1, kZ = 2 };
enum AxisDirection { kNegative = -1, kPositive = 1 };

#endif