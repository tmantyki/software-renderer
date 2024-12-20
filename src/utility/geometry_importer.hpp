#ifndef GEOMETRY_IMPORTER_HPP
#define GEOMETRY_IMPORTER_HPP

#include "geometry/space.hpp"

class GeometryImporter {
 public:
  GeometryImporter(Space& space);
  virtual void ImportGeometryFromFile(const char* filename) = 0;

 protected:
  std::array<Vertex, kVerticesPerTriangle * kMaxTriangles> vertices_;
  Space& space_;
};

class ObjGeometryImporter : public GeometryImporter {
 public:
  ObjGeometryImporter(Space& space);
  virtual void ImportGeometryFromFile(const char* filename) override;
};

#endif