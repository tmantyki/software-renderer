#ifndef COMMON_HPP
#define COMMON_HPP

#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>

#include <SDL2/SDL.h>
#include <Eigen/Core>
#include <Eigen/Geometry>

constexpr float kPi = 3.14159265358979323846264338;
constexpr float kFloatTolerance = 1.0E-3;
constexpr float kAntiZFighting = 1.0E-5;
constexpr size_t kBytesPerPixel = 4;
constexpr size_t kDimensions = 4;
constexpr size_t kSpatialDimensions = 3;
constexpr size_t kUVDimensions = 2;
constexpr size_t kVerticesPerTriangle = 3;
constexpr size_t kViewportDimensions = 3;
constexpr size_t kMaxTriangles = 10000;
constexpr size_t kMaxVertices = kMaxTriangles;
constexpr size_t kNumberOfClippingPlanes = 6;
constexpr size_t kNumberOfPixelChannels = 4;
constexpr uint16_t kWindowWidth = 800;
constexpr uint16_t kWindowHeight = 800;
constexpr bool kClockwiseWinding = true;
constexpr bool kCounterClockwiseWinding = !kClockwiseWinding;
constexpr float kTranslationIncrement = 0.01;
constexpr float kAngularIncrement = 0.01;
constexpr float kViewportRoundingBias = 0.1;
constexpr SDL_PixelFormatEnum kDefaultPixelFormat = SDL_PIXELFORMAT_ARGB8888;

class Point;
class Space;
class Triangle;

typedef std::array<Point, 2> TrianglePlaneIntersections;
typedef std::shared_ptr<Space> SpaceSharedPointer;
typedef std::shared_ptr<Triangle> TriangleSharedPointer;
typedef Eigen::Vector2f Vector2;
typedef Eigen::Vector3f Vector3;
typedef Eigen::Vector4f Vector4;
typedef Eigen::Matrix4f Matrix4;
typedef Eigen::Quaternionf Quaternion;
typedef Eigen::Array<int, kVerticesPerTriangle, Eigen::Dynamic> ClippingMask;
typedef Eigen::Matrix<float, kDimensions, Eigen::Dynamic> NormalMatrix;
typedef Eigen::Matrix<float, kDimensions, Eigen::Dynamic> VertexMatrix;

enum class BoundaryType { kMin, kMax };
enum class TriangleClipMode { kIncludeReference, kExcludeReference };
enum class TriangleHalf { kUpper, kLower };
enum TriangleEdge { kAB = 0, kAC = 1, kBC = 2 };
enum Axis { kX = 0, kY = 1, kZ = 2, kW = 3 };
enum UVDimension { kU = 0, kV = 1 };
enum AxisDirection { kNegative = -1, kNeutral = 0, kPositive = 1 };

#endif