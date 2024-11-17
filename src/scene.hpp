#ifndef SCENE_HPP
#define SCENE_HPP

#include "triangle.hpp"

class Scene {
 public:
  Scene();
  bool AddWorldSpaceTriangle(Triangle& triangle);
  size_t GetWorldSpaceTriangleCount() const;
  const std::array<Triangle*, kMaxTriangles>& GetWorldSpaceTriangles() const;

 private:
  std::array<Triangle*, kMaxTriangles> world_space_triangles_;
  size_t world_space_triangle_count_ = 0;
};

#endif