#include "space.hpp"
#include <array>

// #TODO: refactor helper functions
namespace {
void CopyTriangleColumnsInMatrix(size_t source_index,
                                 size_t destination_index,
                                 VertexMatrix& vertices,
                                 NormalMatrix& normals) {
  assert(source_index < kMaxTriangles);
  assert(destination_index < kMaxTriangles);
  assert(source_index != destination_index);
  size_t src = source_index * 3;
  size_t dst = destination_index * 3;
  std::vector<size_t> src_cols = {src, src + 1, src + 2};
  std::vector<size_t> dst_cols = {dst, dst + 1, dst + 2};
  vertices(Eigen::all, dst_cols) = vertices(Eigen::all, src_cols);
  normals.col(destination_index) = normals.col(source_index);
  // #TODO: investigate if block operations are faster
}

void UpdateMatrixColumnsFromTriangle(size_t destination_index,
                                     Triangle* triangle,
                                     VertexMatrix& vertices,
                                     NormalMatrix& normals) {
  size_t i = destination_index * 3;
  for (size_t k : {0, 1, 2})
    vertices.col(i + k) = triangle->GetVertex(k).GetVector();
  normals.col(destination_index) = triangle->GetNormal();
}
}  // namespace

Space::Space() : triangles_{0} {}

void Space::EnqueueAddTriangle(Triangle& triangle) {
  triangle_add_queue_.push(&triangle);
}

void Space::EnqueueRemoveTriangle(size_t index) {
  assert(index < kMaxTriangles);
  triangle_remove_queue_.push(index);
}

void Space::UpdateSpace() {
  // Calculate net amount of triangles
  size_t last_i = triangle_count_ - 1;
  size_t net_triangle_count = triangle_count_ + triangle_add_queue_.size() -
                              triangle_remove_queue_.size();
  assert(triangle_count_ + triangle_add_queue_.size() >=
         triangle_remove_queue_.size());
  assert(net_triangle_count <= kMaxTriangles);

  // Process items in Remove queue
  while (!triangle_remove_queue_.empty()) {
    size_t i = triangle_remove_queue_.front();
    triangle_remove_queue_.pop();
    assert(triangles_[i]);
    if (!triangle_add_queue_.empty()) {
      triangles_[i] = triangle_add_queue_.front();
      triangle_add_queue_.pop();
      ::UpdateMatrixColumnsFromTriangle(i, triangles_[i], vertices_, normals_);
    } else {
      if (i != last_i) {
        triangles_[i] = triangles_[last_i];
        ::CopyTriangleColumnsInMatrix(last_i, i, vertices_, normals_);
      }
      triangles_[last_i--] = nullptr;
    }
  }

  // Resize matrices
  if (net_triangle_count != triangle_count_) {
    vertices_.conservativeResize(kDimensions, 3 * net_triangle_count);
    normals_.conservativeResize(kDimensions, net_triangle_count);
  }

  // Process remaining items in Add queue
  while (!triangle_add_queue_.empty()) {
    triangles_[++last_i] = triangle_add_queue_.front();
    triangle_add_queue_.pop();
    UpdateMatrixColumnsFromTriangle(last_i, triangles_[last_i], vertices_,
                                    normals_);
  }

  // Update triangle count
  triangle_count_ = net_triangle_count;
}

size_t Space::GetTriangleCount() const {
  return triangle_count_;
}

const std::array<Triangle*, kMaxTriangles>& Space::GetTriangles() const {
  return triangles_;
}

const VertexMatrix& Space::GetVertices() const {
  return vertices_;
}

const NormalMatrix& Space::GetNormals() const {
  return normals_;
}

#include <iostream>
void Space::ClipTriangles(const Plane& plane) {
  Eigen::VectorXf clipping_vector =
      plane.GetVectorNormalized().transpose() * vertices_;
  std::cout << "\n" << clipping_vector << "\n";
  std::cout << "\n" << (clipping_vector.array() >= 0).reshaped(3, 8) << "\n";
  std::cout << "\n"
            << (clipping_vector.array() >= 0)
                   .reshaped(3, 8)
                   .cast<int>()
                   .colwise()
                   .sum()
            << "\n";
}