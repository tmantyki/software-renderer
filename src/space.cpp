#include <array>
#include <memory>

#include "line_segment.hpp"
#include "space.hpp"

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

size_t FindVertexIndexByClipMask(Eigen::Array<int, 3, 1> mask_column,
                                 int match_mask_value) {
  for (size_t index : {0, 1, 2})
    if (mask_column(index) == match_mask_value)
      return index;
  return -1;
}

TrianglePlaneIntersections GetTrianglePlaneIntersections(
    size_t triangle_index,
    size_t single_vertex_index,
    const VertexMatrix& vertices,
    const Plane& plane) {
  size_t a = single_vertex_index + triangle_index * 3;
  size_t b = (a + 1) % 3 + triangle_index * 3;
  size_t c = (a + 2) % 3 + triangle_index * 3;
  LineSegment line_segment_ab(Point(vertices.block(0, a, 3, 1)),
                              Point(vertices.block(0, b, 3, 1)));
  LineSegment line_segment_ac(Point(vertices.block(0, a, 3, 1)),
                              Point(vertices.block(0, c, 3, 1)));
  float ab_t = line_segment_ab.GetPlaneIntersectionParameter(plane);
  float ac_t = line_segment_ac.GetPlaneIntersectionParameter(plane);
  return std::array<Point, 2>{line_segment_ab.GetInterpolatedPoint(ab_t),
                              line_segment_ac.GetInterpolatedPoint(ac_t)};
}

void AddSubstituteTriangles(const Triangle& triangle,
                            size_t single_vertex_index,
                            TriangleClipMode clip_mode,
                            const TrianglePlaneIntersections& intersections,
                            std::vector<TriangleSharedPointer>& substitutes) {
  Direction normal(triangle.GetNormal()({0, 1, 2}));
  Vertex vertex_ab(intersections[TriangleEdge::kAB].GetVector()({0, 1, 2}));
  Vertex vertex_ac(intersections[TriangleEdge::kAC].GetVector()({0, 1, 2}));
  if (clip_mode == TriangleClipMode::kIncludeReference) {
    Vertex vertex_0 = triangle.GetVertex(single_vertex_index);
    substitutes.push_back(
        std::make_shared<Triangle>(vertex_0, vertex_ab, vertex_ac, normal));
  } else if (clip_mode == TriangleClipMode::kExcludeReference) {
    Vertex vertex_1 = triangle.GetVertex((single_vertex_index + 1) % 3);
    Vertex vertex_2 = triangle.GetVertex((single_vertex_index + 2) % 3);
    substitutes.push_back(
        std::make_shared<Triangle>(vertex_ab, vertex_1, vertex_ac, normal));
    substitutes.push_back(
        std::make_shared<Triangle>(vertex_ac, vertex_1, vertex_2, normal));
  }
}
}  // namespace

Space::Space() : triangles_{0} {}

void Space::EnqueueAddTriangle(TriangleSharedPointer triangle_ptr) {
  triangle_add_queue_.push(triangle_ptr);
}

void Space::EnqueueAddMultipleTriangles(
    std::vector<TriangleSharedPointer> triangles) {
  for (TriangleSharedPointer tr : triangles)
    triangle_add_queue_.push(tr);
}

void Space::EnqueueRemoveTriangle(size_t index) {
  assert(index < kMaxTriangles);
  triangle_remove_queue_.push(index);
}

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

std::vector<TriangleSharedPointer> Space::GetClipSubstitutes(
    size_t triangle_index,
    const Plane& plane,
    size_t single_vertex_index,
    TriangleClipMode clip_mode) const {
  TrianglePlaneIntersections intersections = ::GetTrianglePlaneIntersections(
      triangle_index, single_vertex_index, vertices_, plane);
  std::vector<TriangleSharedPointer> substitutes;
  // #TODO: vertex attributes
  ::AddSubstituteTriangles(*triangles_[triangle_index], single_vertex_index,
                           clip_mode, intersections, substitutes);
  return substitutes;
}

void Space::ClipAllTriangles(const Plane& plane) {
  ClippingMask clipping_mask = GenerateClippingMask(plane);
  ProcessClippingMask(clipping_mask, plane);
  UpdateSpace();
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

ClippingMask Space::GenerateClippingMask(const Plane& plane) const {
  return ((plane.GetVectorNormalized().transpose() * vertices_).array() >= 0)
      .reshaped(kVerticesPerTriangle, triangle_count_)
      .cast<int>();
}

void Space::ProcessClippingMask(const ClippingMask& clipping_mask,
                                const Plane& plane) {
  Eigen::Array<int, 1, Eigen::Dynamic> mask_cols_sums =
      clipping_mask.colwise().sum();
  for (size_t col = 0; col < static_cast<size_t>(clipping_mask.cols()); col++) {
    int match_mask_value = 0;
    TriangleClipMode clip_mode = TriangleClipMode::kExcludeReference;
    if (mask_cols_sums(col) == 3)
      continue;
    EnqueueRemoveTriangle(col);
    if (mask_cols_sums(col) == 0) {
      continue;
    } else if (mask_cols_sums(col) == 1) {
      clip_mode = TriangleClipMode::kIncludeReference;
      match_mask_value = 1;
    }
    size_t single_vertex_index =
        ::FindVertexIndexByClipMask(clipping_mask.col(col), match_mask_value);
    EnqueueAddMultipleTriangles(
        GetClipSubstitutes(col, plane, single_vertex_index, clip_mode));
  }
}
