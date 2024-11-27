#include <array>
#include <memory>

#include "line_segment.hpp"
#include "space.hpp"

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
                                     TriangleSharedPointer triangle,
                                     VertexMatrix& vertices,
                                     NormalMatrix& normals) {
  size_t i = destination_index * 3;
  for (size_t k : {0, 1, 2})
    vertices.col(i + k) = triangle->GetVertex(k).GetVector();
  normals.col(destination_index) = triangle->GetNormal();
}
}  // namespace

Space::Space() : triangles_{0} {}

void Space::EnqueueAddTriangle(TriangleSharedPointer triangle_ptr) {
  triangle_add_queue_.push(triangle_ptr);
}

void Space::EnqueueRemoveTriangle(size_t index) {
  assert(index < kMaxTriangles);
  triangle_remove_queue_.push(index);
}

#include <iostream> // #TODO: remove

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
    std::cout << "Debug_removing: i: " << i << " last_i: " << last_i << "\n";
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

const std::array<TriangleSharedPointer, kMaxTriangles>& Space::GetTriangles()
    const {
  return triangles_;
}

const VertexMatrix& Space::GetVertices() const {
  return vertices_;
}

const NormalMatrix& Space::GetNormals() const {
  return normals_;
}

SpaceSharedPointer Space::ClipTriangles(const Plane& plane) {
  Eigen::Array<int, 1, Eigen::Dynamic> clipping_array =
      (((plane.GetVectorNormalized().transpose() * vertices_).array() >= 0)
           .reshaped(3, 8)
           .cast<int>()
           .colwise() *
       Eigen::Vector3i{1, 2, 4}.array())
          .colwise()
          .sum();
  std::cout << "\n" << clipping_array << "\n\n";
  SpaceSharedPointer post_clip_space = std::make_shared<Space>(*this);
  size_t k = 0;
  for (int i : clipping_array) {
    std::vector<std::shared_ptr<Triangle>> new_triangles;
    switch (i) {
      case 0:  // All vertices are outside
        break;
      case 1:  // Vertex 1 is inside
        new_triangles =
            ClipTriangle(k, plane, 0, TriangleClipMode::kIncludePivot);
        break;
      case 2:  // Vertex 2 is inside
        new_triangles =
            ClipTriangle(k, plane, 1, TriangleClipMode::kIncludePivot);
        break;
      case 3:  // Vertex 3 is outside
        new_triangles =
            ClipTriangle(k, plane, 2, TriangleClipMode::kExcludePivot);
        break;
      case 4:  // Vertex 3 is inside
        new_triangles =
            ClipTriangle(k, plane, 2, TriangleClipMode::kIncludePivot);
        break;
      case 5:  // Vertex 2 is outside
        new_triangles =
            ClipTriangle(k, plane, 1, TriangleClipMode::kExcludePivot);
        break;
      case 6:  // Vertex 1 is outside
        new_triangles =
            ClipTriangle(k, plane, 0, TriangleClipMode::kExcludePivot);
        break;
      case 7:  // All vertices are inside
        break;
    }
    // for (std::shared_ptr<Triangle> t : new_triangles) {
    //   auto x = &(*t);
    //   post_clip_space->EnqueueAddTriangle(*t));
    // }
    if (i != 7)
      post_clip_space->EnqueueRemoveTriangle(k);
    k++;
  }
  post_clip_space->UpdateSpace();
  return post_clip_space;
}

std::vector<std::shared_ptr<Triangle>> Space::ClipTriangle(
    size_t triangle_index,
    const Plane& plane,
    size_t pivot,
    TriangleClipMode clip_mode) {
  size_t i = triangle_index;
  size_t a = pivot;
  size_t b = (a + 1) % 3;
  size_t c = (a + 2) % 3;
  a += i * 3;
  b += i * 3;
  c += i * 3;
  LineSegment line_segment_ab(Point(vertices_.block(0, a, 3, 1)),
                              Point(vertices_.block(0, b, 3, 1)));
  LineSegment line_segment_ac(Point(vertices_.block(0, a, 3, 1)),
                              Point(vertices_.block(0, c, 3, 1)));
  std::vector<std::shared_ptr<Triangle>> new_triangles;
  float ab_t = line_segment_ab.GetPlaneIntersectionParameter(plane);
  float ac_t = line_segment_ac.GetPlaneIntersectionParameter(plane);
  Point ab_intersection = line_segment_ab.GetInterpolatedPoint(ab_t);
  Point ac_intersection = line_segment_ac.GetInterpolatedPoint(ac_t);
  // #TODO: refactor Point and Vertex constructors etc.
  // #TODO: normals
  // #TODO: vertex attributes
  Vertex vertex_ab(ab_intersection.GetVector()({0, 1, 2}));
  Vertex vertex_ac(ac_intersection.GetVector()({0, 1, 2}));
  if (clip_mode == TriangleClipMode::kIncludePivot) {
    Vertex vertex_0 = triangles_[triangle_index]->GetVertex(pivot);
    std::shared_ptr<Triangle> triangle =
        std::make_shared<Triangle>(vertex_0, vertex_ab, vertex_ac);
    new_triangles.push_back(triangle);
  } else if (clip_mode == TriangleClipMode::kExcludePivot) {
    Vertex vertex_1 = triangles_[triangle_index]->GetVertex((pivot + 1) % 3);
    Vertex vertex_2 = triangles_[triangle_index]->GetVertex((pivot + 2) % 3);
    std::shared_ptr<Triangle> triangle_1 =
        std::make_shared<Triangle>(vertex_ab, vertex_1, vertex_ac);
    std::shared_ptr<Triangle> triangle_2 =
        std::make_shared<Triangle>(vertex_ac, vertex_1, vertex_2);
    new_triangles.push_back(triangle_1);
    new_triangles.push_back(triangle_2);
  }
  return new_triangles;
}