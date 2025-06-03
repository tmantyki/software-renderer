#pragma once

#include <queue>
#include "triangle.hpp"

class Space {
 public:
  Space() : triangles_{0} {}
  void EnqueueAddTriangle(TriangleSharedPointer triangle_ptr) {
    triangle_add_queue_.push(triangle_ptr);
  }
  void EnqueueAddMultipleTriangles(
      std::vector<TriangleSharedPointer> triangles) {
    for (TriangleSharedPointer tr : triangles)
      triangle_add_queue_.push(tr);
  }
  void EnqueueRemoveTriangle(size_t index) {
    assert(index < kMaxTriangles);
    triangle_remove_queue_.push(index);
  }
  void UpdateSpace();
  size_t GetTriangleCount() const { return triangle_count_; }
  const std::array<TriangleSharedPointer, kMaxTriangles>& GetTriangles() const {
    return triangles_;
  }
  const VertexMatrix& GetVertices() const { return vertices_; }
  const NormalMatrix& GetNormals() const { return normals_; }
  std::vector<TriangleSharedPointer> GetHomogeneousClipSubstitutes(
      size_t triangle_index,
      size_t solo_vertex,
      Axis axis,
      AxisDirection axis_direction,
      TriangleClipMode clip_mode) const;
  void ClipAllTriangles(Axis axis, AxisDirection axis_direction);
  void TransformVertices(const Matrix4& transformation) {
    vertices_ = transformation * vertices_;
  }
  void TransformNormals(const Matrix4& transformation) {
    normals_ = transformation * normals_;
  }
  void Dehomogenize() {
    vertices_ =
        (vertices_.array().rowwise() / vertices_.row(3).array()).matrix();
  }

 private:
  std::array<TriangleSharedPointer, kMaxTriangles> triangles_;
  std::queue<TriangleSharedPointer> triangle_add_queue_;
  std::queue<size_t> triangle_remove_queue_;
  size_t triangle_count_ = 0;
  VertexMatrix vertices_;
  NormalMatrix normals_;

  struct UpdateSpaceParameters {
    size_t initial_triangle_count;
    size_t final_triangle_count;
    size_t add_queue_size;
    size_t remove_queue_size;
  };

  void InitializeUpdateSpaceParameters(
      struct UpdateSpaceParameters& parameters);
  void ReplaceRemovedWithAdded(struct UpdateSpaceParameters& parameters);
  void DefragmentVectorAndMatrices(struct UpdateSpaceParameters& parameters);
  void ResizeVectorAndMatrices(struct UpdateSpaceParameters& parameters);
  void AddRemainingInQueue(struct UpdateSpaceParameters& parameters);
  ClippingMask HomogeneousClippingMask(Axis axis,
                                       AxisDirection axis_direction) const;
  void ProcessHomogeneousClippingMask(const ClippingMask& clipping_mask,
                                      Axis axis,
                                      AxisDirection axis_direction);
  void GetInterpolatedVertices(size_t triangle_index,
                               size_t single_vertex_index,
                               Axis axis,
                               AxisDirection axis_direction,
                               InterpolatedVertex& iv_ab,
                               InterpolatedVertex& iv_ac) const;
};
