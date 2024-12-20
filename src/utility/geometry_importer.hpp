#ifndef GEOMETRY_IMPORTER_HPP
#define GEOMETRY_IMPORTER_HPP

#include "geometry/space.hpp"

class GeometryImporter {
 public:
  GeometryImporter(Space& space);
  virtual bool ImportGeometryFromFile(const char* filename) = 0;

 protected:
  std::array<Vertex, kMaxVertices> vertices_;
  Space& space_;
};

class ObjGeometryImporter : public GeometryImporter {
 public:
  ObjGeometryImporter(Space& space);
  virtual bool ImportGeometryFromFile(const char* filename) override;

 private:
  bool ParseLine(const std::string& line);
  bool ParseVertex(std::stringstream& vertex_params);
  size_t triangle_counter_;
  size_t vertex_counter_;
};

#endif