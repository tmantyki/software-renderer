#ifndef SPACE_HPP
#define SPACE_HPP

#include <queue>
#include "triangle.hpp"

typedef Eigen::Matrix<float,
                      kDimensions,
                      Eigen::Dynamic,
                      0,
                      kDimensions,
                      3 * kMaxTriangles>
    VertexMatrix;
typedef Eigen::
    Matrix<float, kDimensions, Eigen::Dynamic, 0, kDimensions, kMaxTriangles>
        NormalMatrix;

class Space;
typedef std::shared_ptr<Triangle> TriangleSharedPointer;
typedef std::shared_ptr<Space> SpaceSharedPointer;

enum class TriangleClipMode { kIncludePivot, kExcludePivot };

class Space {
 public:
  Space();
  void EnqueueAddTriangle(TriangleSharedPointer triangle_ptr);
  void EnqueueRemoveTriangle(size_t index);
  void UpdateSpace();
  size_t GetTriangleCount() const;
  const std::array<TriangleSharedPointer, kMaxTriangles>& GetTriangles() const;
  const VertexMatrix& GetVertices() const;
  const NormalMatrix& GetNormals() const;
  SpaceSharedPointer ClipTriangles(const Plane& plane);
  std::vector<TriangleSharedPointer> ClipTriangle(size_t triangle_index,
                                                  const Plane& plane,
                                                  size_t pivot,
                                                  TriangleClipMode clip_mode);

 private:
  std::array<TriangleSharedPointer, kMaxTriangles> triangles_;
  std::queue<TriangleSharedPointer> triangle_add_queue_;
  std::queue<size_t> triangle_remove_queue_;
  size_t triangle_count_ = 0;
  VertexMatrix vertices_;
  NormalMatrix normals_;
};

#endif