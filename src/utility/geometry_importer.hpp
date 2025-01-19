#ifndef GEOMETRY_IMPORTER_HPP
#define GEOMETRY_IMPORTER_HPP

#include "geometry/space.hpp"

class GeometryImportException : public std::exception {
 public:
  GeometryImportException(const char* error_message);
  virtual const char* what() const noexcept override;

 private:
  const char* error_message_;
};

class InvalidFileException : public GeometryImportException {
 public:
  InvalidFileException(const char* error_message);
};

class UnknownCommandException : public GeometryImportException {
 public:
  UnknownCommandException(const char* error_message);
};

class WorldLimitsExceededException : public GeometryImportException {
 public:
  WorldLimitsExceededException(const char* error_message);
};

class MalformedParametersException : public GeometryImportException {
 public:
  MalformedParametersException(const char* error_message);
};

class UnsupportedPrimitiveException : public GeometryImportException {
 public:
  UnsupportedPrimitiveException(const char* error_message);
};

class GeometryImporter {
 public:
  GeometryImporter(Space& space);
  void ImportGeometryFromFile(const char* filename);
  virtual void ImportGeometryFromInputStream(std::istream& input_stream) = 0;
  size_t GetTriangleCount() const noexcept;
  size_t GetVertexCount() const noexcept;

 protected:
  std::array<Vertex, kMaxVertices> vertices_;
  std::array<UVCoordinate, kMaxVertices> uv_coordinates_;
  Space& space_;
  size_t triangle_counter_;
  size_t vertex_counter_;
  size_t uv_counter_;
};

class ObjGeometryImporter : public GeometryImporter {
 public:
  ObjGeometryImporter(Space& space);
  virtual void ImportGeometryFromInputStream(
      std::istream& input_stream) override;

 private:
  void ParseLine(const std::string& line);
  void ParseVertex(std::stringstream& vertex_params);
  void ParseFace(std::stringstream& face_params);
  void ParseUVCoordinate(std::stringstream& uv_params);
};

#endif