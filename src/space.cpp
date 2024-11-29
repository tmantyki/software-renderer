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

#include <iostream>  // #TODO: remove

void Space::UpdateSpace() {
  struct UpdateSpaceParameters update_parameters;
  InitializeUpdateSpaceParameters(update_parameters);
  ReplaceRemovedWithAdded(update_parameters);
  DefragmentVectorAndMatrices(update_parameters);
  ResizeVectorAndMatrices(update_parameters);
  AddRemainingInQueue(update_parameters);
  triangle_count_ = update_parameters.final_triangle_count;
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
  if (clip_mode == TriangleClipMode::kIncludeReference) {
    Vertex vertex_0 = triangles_[triangle_index]->GetVertex(pivot);
    std::shared_ptr<Triangle> triangle =
        std::make_shared<Triangle>(vertex_0, vertex_ab, vertex_ac);
    new_triangles.push_back(triangle);
  } else if (clip_mode == TriangleClipMode::kExcludeReference) {
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

SpaceSharedPointer Space::ClipAllTriangles(const Plane& plane) {
  Eigen::Array<int, 1, Eigen::Dynamic> clipping_array =
      (((plane.GetVectorNormalized().transpose() * vertices_).array() >= 0)
           .reshaped(kVerticesPerTriangle, triangle_count_)
           .cast<int>()
           .colwise() *
       Eigen::Vector3i{1, 2, 4}.array())
          .colwise()
          .sum();
  std::cout << "\n" << clipping_array << "\n\n";
  SpaceSharedPointer clipped_space = std::make_shared<Space>(*this);
  size_t k = 0;
  for (int i : clipping_array) {
    std::vector<std::shared_ptr<Triangle>> new_triangles;
    switch (i) {
      case 0:  // All vertices are outside
        break;
      case 1:  // Vertex 1 is inside
        new_triangles =
            ClipTriangle(k, plane, 0, TriangleClipMode::kIncludeReference);
        break;
      case 2:  // Vertex 2 is inside
        new_triangles =
            ClipTriangle(k, plane, 1, TriangleClipMode::kIncludeReference);
        break;
      case 3:  // Vertex 3 is outside
        new_triangles =
            ClipTriangle(k, plane, 2, TriangleClipMode::kExcludeReference);
        break;
      case 4:  // Vertex 3 is inside
        new_triangles =
            ClipTriangle(k, plane, 2, TriangleClipMode::kIncludeReference);
        break;
      case 5:  // Vertex 2 is outside
        new_triangles =
            ClipTriangle(k, plane, 1, TriangleClipMode::kExcludeReference);
        break;
      case 6:  // Vertex 1 is outside
        new_triangles =
            ClipTriangle(k, plane, 0, TriangleClipMode::kExcludeReference);
        break;
      case 7:  // All vertices are inside
        break;
    }
    for (std::shared_ptr<Triangle> t : new_triangles) {
      clipped_space->EnqueueAddTriangle(t);
    }
    if (i != 7)
      clipped_space->EnqueueRemoveTriangle(k);
    k++;
  }
  clipped_space->UpdateSpace();
  return clipped_space;
}

void Space::InitializeUpdateSpaceParameters(
    struct UpdateSpaceParameters& parameters) {
  parameters.initial_triangle_count = triangle_count_;
  parameters.add_queue_size = triangle_add_queue_.size();
  parameters.remove_queue_size = triangle_remove_queue_.size();
  parameters.final_triangle_count = parameters.initial_triangle_count +
                                    parameters.add_queue_size -
                                    parameters.remove_queue_size;
  assert(parameters.initial_triangle_count + parameters.add_queue_size >=
         parameters.remove_queue_size);
  assert(parameters.final_triangle_count <= kMaxTriangles);
}

void Space::ReplaceRemovedWithAdded(struct UpdateSpaceParameters& parameters) {
  while (!triangle_remove_queue_.empty()) {
    size_t i = triangle_remove_queue_.front();
    triangle_remove_queue_.pop();
    assert(triangles_[i]);
    if (!triangle_add_queue_.empty()) {
      triangles_[i] = triangle_add_queue_.front();
      triangle_add_queue_.pop();
      ::UpdateMatrixColumnsFromTriangle(i, triangles_[i], vertices_, normals_);
    } else {
      assert(parameters.final_triangle_count <
             parameters.initial_triangle_count);
      assert(parameters.remove_queue_size > parameters.add_queue_size);
      triangles_[i] = nullptr;
    }
  }
}

void Space::DefragmentVectorAndMatrices(
    struct UpdateSpaceParameters& parameters) {
  if (parameters.final_triangle_count >= parameters.initial_triangle_count)
    return;
  auto begin = triangles_.begin();
  auto rbegin = triangles_.rbegin();
  auto end = triangles_.end();
  auto rend = triangles_.rend();
  auto is_not_nullptr = [](TriangleSharedPointer& t) { return t != nullptr; };
  for (size_t k =
           parameters.initial_triangle_count - parameters.final_triangle_count;
       k > 0; k--) {
    auto it_null = std::find(begin, end, nullptr);
    auto it_valid = std::find_if(rbegin, rend, is_not_nullptr);
    if (it_null > it_valid.base() - 2)
      break;
    size_t src_i = triangles_.rend() - it_valid - 1;
    size_t dst_i = it_null - triangles_.begin();
    *it_null = *it_valid;
    *it_valid = nullptr;
    ::CopyTriangleColumnsInMatrix(src_i, dst_i, vertices_, normals_);
    begin = it_null + 1;
    rbegin = it_valid + 1;
  }
}

void Space::ResizeVectorAndMatrices(struct UpdateSpaceParameters& parameters) {
  if (parameters.final_triangle_count == parameters.initial_triangle_count)
    return;
  vertices_.conservativeResize(kDimensions,
                               3 * parameters.final_triangle_count);
  normals_.conservativeResize(kDimensions, parameters.final_triangle_count);
}

void Space::AddRemainingInQueue(struct UpdateSpaceParameters& parameters) {
  size_t last_i = parameters.initial_triangle_count - 1;
  while (!triangle_add_queue_.empty()) {
    triangles_[++last_i] = triangle_add_queue_.front();
    triangle_add_queue_.pop();
    UpdateMatrixColumnsFromTriangle(last_i, triangles_[last_i], vertices_,
                                    normals_);
  }
}
