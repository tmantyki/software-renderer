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
  std::vector<TriangleSharedPointer> GetClipSubstitutes(
      size_t triangle_index,
      const Plane& plane,
      size_t solo_vertex,
      TriangleClipMode clip_mode) const;
  void ClipAllTriangles(const Plane& plane);

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
  ClippingMask GenerateClippingMask(const Plane& plane) const;
  void ProcessClippingMask(const ClippingMask& clipping_mask,
                           const Plane& plane);
};

#endif