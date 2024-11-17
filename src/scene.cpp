#include "scene.hpp"
#include <array>

Scene::Scene() : world_space_triangles_{0} {}

bool Scene::AddWorldSpaceTriangle(Triangle& triangle) {
  world_space_triangles_[world_space_triangle_count_++] = &triangle;
  return true;
}

size_t Scene::GetWorldSpaceTriangleCount() const {
  return world_space_triangle_count_;
}

const std::array<Triangle*, kMaxTriangles>& Scene::GetWorldSpaceTriangles()
    const {
  return world_space_triangles_;
}