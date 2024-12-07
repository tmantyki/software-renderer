#ifndef SPACE_HPP
#define SPACE_HPP

#include <queue>
#include "triangle.hpp"

class Space {
 public:
  Space();
  void EnqueueAddTriangle(TriangleSharedPointer triangle_ptr);
  void EnqueueAddMultipleTriangles(
      std::vector<TriangleSharedPointer> triangles);
  void EnqueueRemoveTriangle(size_t index);
  void UpdateSpace();
  size_t GetTriangleCount() const;
  const std::array<TriangleSharedPointer, kMaxTriangles>& GetTriangles() const;
  const VertexMatrix& GetVertices() const;
  const NormalMatrix& GetNormals() const;
  std::vector<TriangleSharedPointer> GetHomogenousClipSubstitutes(
      size_t triangle_index,
      size_t solo_vertex,
      Axis axis,
      AxisDirection axis_direction,
      TriangleClipMode clip_mode) const;
  void ClipAllTriangles(Axis axis, AxisDirection axis_direction);
  void TransformVertices(const Eigen::Matrix4f& transformation);
  void TransformNormals(const Eigen::Matrix4f& transformation);
  void Dehomogenize();

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
  ClippingMask HomogenousClippingMask(Axis axis,
                                      AxisDirection axis_direction) const;
  void ProcessHomogenousClippingMask(const ClippingMask& clipping_mask,
                                     Axis axis,
                                     AxisDirection axis_direction);
  TrianglePlaneIntersections GetTrianglePlaneIntersections(
      size_t triangle_index,
      size_t single_vertex_index,
      Axis axis,
      AxisDirection axis_direction) const;
};

#endif