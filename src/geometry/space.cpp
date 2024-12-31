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

// #TODO: refactor
void AddSubstituteTriangles(const VertexMatrix& vertices,
                            const NormalMatrix& normals,
                            const Triangle& triangle,
                            size_t triangle_index,
                            size_t single_vertex_index,
                            TriangleClipMode clip_mode,
                            TrianglePlaneIntersections& intersections,
                            std::vector<TriangleSharedPointer>& substitutes) {
  size_t column;
  (void)normals;  // #TODO: finish normal implementation!!
  Direction normal(triangle.GetNormal());
  Vertex vertex_ab(intersections[TriangleEdge::kAB]);
  Vertex vertex_ac(intersections[TriangleEdge::kAC]);
  if (clip_mode == TriangleClipMode::kIncludeReference) {
    column = triangle_index * 3 + single_vertex_index;
    Vertex vertex_0 = Vertex(Vector4(vertices.col(column)));
    substitutes.push_back(
        std::make_shared<Triangle>(vertex_0, vertex_ab, vertex_ac, normal));
  } else if (clip_mode == TriangleClipMode::kExcludeReference) {
    column = triangle_index * 3 + ((single_vertex_index + 1) % 3);
    Vertex vertex_1 = Vertex(Vector4(vertices.col(column)));
    column = triangle_index * 3 + ((single_vertex_index + 2) % 3);
    Vertex vertex_2 = Vertex(Vector4(vertices.col(column)));
    substitutes.push_back(
        std::make_shared<Triangle>(vertex_ab, vertex_1, vertex_ac, normal));
    substitutes.push_back(
        std::make_shared<Triangle>(vertex_ac, vertex_1, vertex_2, normal));
  }
}

float HomogeneousInterpolation(Vector4 vector_a,
                               Vector4 vector_b,
                               Axis axis,
                               AxisDirection axis_direction) {
  float a_val = vector_a[axis];
  float b_val = vector_b[axis];
  float a_w = axis_direction * vector_a[kW];
  float b_w = axis_direction * vector_b[kW];
  assert(a_val - a_w - b_val + b_w != 0); // #TODO: investigate
  return (a_val - a_w) / (a_val - a_w - b_val + b_w);
}

bool SortClipVertices(const Vector4& lhs, const Vector4& rhs) noexcept {
  if (lhs(kX) == rhs(kX)) {
    if (lhs(kY) == rhs(kY)) {
      if (lhs(kZ) == rhs(kZ)) {
        return lhs(kW) < rhs(kW);
      }
      return lhs(kZ) < rhs(kZ);
    }
    return lhs(kY) < rhs(kY);
  }
  return lhs(kX) < rhs(kX);
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

std::vector<TriangleSharedPointer> Space::GetHomogeneousClipSubstitutes(
    size_t triangle_index,
    size_t single_vertex_index,
    Axis axis,
    AxisDirection axis_direction,
    TriangleClipMode clip_mode) const {
  TrianglePlaneIntersections intersections = GetTrianglePlaneIntersections(
      triangle_index, single_vertex_index, axis, axis_direction);
  std::vector<TriangleSharedPointer> substitutes;
  // #TODO: vertex attributes
  ::AddSubstituteTriangles(vertices_, normals_, *triangles_[triangle_index],
                           triangle_index, single_vertex_index, clip_mode,
                           intersections, substitutes);
  return substitutes;
}

void Space::ClipAllTriangles(Axis axis, AxisDirection axis_direction) {
  ClippingMask clipping_mask = HomogeneousClippingMask(axis, axis_direction);
  ProcessHomogeneousClippingMask(clipping_mask, axis, axis_direction);
  UpdateSpace();
}

void Space::TransformVertices(const Matrix4& transformation) {
  vertices_ = transformation * vertices_;
}

void Space::TransformNormals(const Matrix4& transformation) {
  normals_ = transformation * normals_;
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
      (void)parameters;  // required for Release build
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

ClippingMask Space::HomogeneousClippingMask(
    Axis axis,
    AxisDirection axis_direction) const {
  return (axis_direction * vertices_.row(axis).array() <=
          (vertices_.row(kW)).array())
      .reshaped(kVerticesPerTriangle, triangle_count_)
      .cast<int>();
}

void Space::ProcessHomogeneousClippingMask(const ClippingMask& clipping_mask,
                                           Axis axis,
                                           AxisDirection axis_direction) {
  Eigen::Array<int, 1, Eigen::Dynamic> mask_cols_sums =
      clipping_mask.colwise().sum();
  // #TODO: can be parallellized
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
    EnqueueAddMultipleTriangles(GetHomogeneousClipSubstitutes(
        col, single_vertex_index, axis, axis_direction, clip_mode));
  }
}

// #TODO: refactor!
TrianglePlaneIntersections Space::GetTrianglePlaneIntersections(
    size_t triangle_index,
    size_t single_vertex_index,
    Axis axis,
    AxisDirection axis_direction) const {
  size_t a = single_vertex_index + triangle_index * 3;
  size_t b = (a + 1) % 3 + triangle_index * 3;
  size_t c = (a + 2) % 3 + triangle_index * 3;
  Vector4 vector_a = vertices_.block(0, a, kDimensions, 1);
  Vector4 vector_b = vertices_.block(0, b, kDimensions, 1);
  Vector4 vector_c = vertices_.block(0, c, kDimensions, 1);
  float ab_t = ::HomogeneousInterpolation(
      std::min({vector_a, vector_b}, ::SortClipVertices),
      std::max({vector_a, vector_b}, ::SortClipVertices), axis, axis_direction);
  float ac_t = ::HomogeneousInterpolation(
      std::min({vector_a, vector_c}, ::SortClipVertices),
      std::max({vector_a, vector_c}, ::SortClipVertices), axis, axis_direction);
  Vector4 interpolated_ab =
      std::min({vector_a, vector_b}, ::SortClipVertices) * (1 - ab_t) +
      std::max({vector_a, vector_b}, ::SortClipVertices) * ab_t;
  Vector4 interpolated_ac =
      std::min({vector_a, vector_c}, ::SortClipVertices) * (1 - ac_t) +
      std::max({vector_a, vector_c}, ::SortClipVertices) * ac_t;

  Point point_interpolated_ab(interpolated_ab);
  Point point_interpolated_ac(interpolated_ac);
  return std::array<Point, 2>{point_interpolated_ab, point_interpolated_ac};
}

void Space::Dehomogenize() {
  vertices_ = (vertices_.array().rowwise() / vertices_.row(3).array()).matrix();
}