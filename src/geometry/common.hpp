#ifndef COMMON_HPP
#define COMMON_HPP

#include <cassert>
#include <cstddef>
#include <memory>

constexpr float kPi = 3.14159265358979323846264338;
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

#endif