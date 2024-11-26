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

class Space {
 public:
  Space();
  void EnqueueAddTriangle(Triangle& triangle);
  void EnqueueRemoveTriangle(size_t index);
  void UpdateSpace();
  size_t GetTriangleCount() const;
  const std::array<Triangle*, kMaxTriangles>& GetTriangles() const;
  const VertexMatrix& GetVertices() const;
  const NormalMatrix& GetNormals() const;
  void ClipTriangles(const Plane& plane);

 private:
  std::array<Triangle*, kMaxTriangles> triangles_;
  std::queue<Triangle*> triangle_add_queue_;
  std::queue<size_t> triangle_remove_queue_;
  size_t triangle_count_ = 0;
  VertexMatrix vertices_;
  NormalMatrix normals_;
};

#endif