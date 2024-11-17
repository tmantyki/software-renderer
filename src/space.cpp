#include "space.hpp"
#include <array>

namespace {
void CopyTriangleColumnsInMatrix(size_t source_index,
                                 size_t destination_index,
                                 VertexMatrix& matrix) {
  assert(source_index < kMaxTriangles);
  assert(destination_index < kMaxTriangles);
  assert(source_index != destination_index);
  size_t src = source_index * 3;
  size_t dst = destination_index * 3;
  std::vector<size_t> src_cols = {src, src + 1, src + 2};
  std::vector<size_t> dst_cols = {dst, dst + 1, dst + 2};
  matrix(Eigen::all, dst_cols) = matrix(Eigen::all, src_cols);
  // #TODO: investigate if block operations are faster
}

void UpdateMatrixColumnsFromTriangle(size_t destination_index,
                                     Triangle* triangle,
                                     VertexMatrix& matrix) {
  size_t i = destination_index * 3;
  for (size_t k : {0, 1, 2})
    matrix.col(i + k) = triangle->GetVertex(k).GetVector();
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

size_t Space::GetTriangleCount() const {
  return triangle_count_;
}

const std::array<Triangle*, kMaxTriangles>& Space::GetTriangles() const {
  return triangles_;
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
      ::UpdateMatrixColumnsFromTriangle(i, triangles_[i], vertices_);
    } else {
      if (i != last_i) {
        triangles_[i] = triangles_[last_i];
        ::CopyTriangleColumnsInMatrix(last_i, i, vertices_);
      }
      triangles_[last_i--] = nullptr;
    }
  }

  // Resize vertex matrix
  if (net_triangle_count != triangle_count_) {
    vertices_.conservativeResize(kDimensions, 3 * net_triangle_count);
  }

  // Process remaining items in Add queue
  while (!triangle_add_queue_.empty()) {
    triangles_[++last_i] = triangle_add_queue_.front();
    triangle_add_queue_.pop();
    UpdateMatrixColumnsFromTriangle(last_i, triangles_[last_i], vertices_);
  }

  // Update triangle count
  triangle_count_ = net_triangle_count;
}

const VertexMatrix& Space::GetVertices() const {
  return vertices_;
}