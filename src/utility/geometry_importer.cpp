#include <fstream>
#include <iostream>

#include "geometry/common.hpp"
#include "geometry/vertex.hpp"
#include "utility/geometry_importer.hpp"

namespace {
bool MoreWordsRemaining(std::stringstream& parameter_stringstream) noexcept {
  parameter_stringstream >> std::ws;
  return !parameter_stringstream.eof();
}

void RearrangeIndicesByWindingDirection(size_t* indices) noexcept {
  if (kClockwiseWinding) {
    size_t tmp = indices[1];
    indices[1] = indices[2];
    indices[2] = tmp;
  }
}
}  // namespace

GeometryImportException::GeometryImportException(const char* error_message)
    : error_message_(error_message) {}

const char* GeometryImportException::what() const noexcept {
  return error_message_;
}

InvalidFileException::InvalidFileException(const char* error_message)
    : GeometryImportException(error_message) {}

UnknownCommandException::UnknownCommandException(const char* error_message)
    : GeometryImportException(error_message) {}

WorldLimitsExceededException::WorldLimitsExceededException(
    const char* error_message)
    : GeometryImportException(error_message) {}

MalformedParametersException::MalformedParametersException(
    const char* error_message)
    : GeometryImportException(error_message) {}

UnsupportedPrimitiveException::UnsupportedPrimitiveException(
    const char* error_message)
    : GeometryImportException(error_message) {}

GeometryImporter::GeometryImporter(Space& space)
    : space_(space), triangle_counter_(0), vertex_counter_(0), uv_counter_(0) {}

void GeometryImporter::ImportGeometryFromFile(const char* filename) {
  std::ifstream input_file_stream;
  input_file_stream.open(filename);
  if (!input_file_stream)
    throw InvalidFileException("File could not be opened!");
  ImportGeometryFromInputStream(input_file_stream);
  input_file_stream.close();
  std::cout << "Successfully read " << vertex_counter_ << " vertices and "
            << triangle_counter_ << " triangles from " << filename << ".\n";
}

size_t GeometryImporter::GetTriangleCount() const noexcept {
  return triangle_counter_;
}

size_t GeometryImporter::GetVertexCount() const noexcept {
  return vertex_counter_;
}

ObjGeometryImporter::ObjGeometryImporter(Space& space)
    : GeometryImporter(space) {}

void ObjGeometryImporter::ImportGeometryFromInputStream(
    std::istream& input_stream) {
  std::string line;
  while (std::getline(input_stream, line))
    ParseLine(line);
  space_.UpdateSpace();
}

void ObjGeometryImporter::ParseLine(const std::string& line) {
  std::stringstream ss(line);
  ss >> std::ws;
  std::string word;
  if (std::getline(ss, word, ' ')) {
    if (word[0] == '#')
      return;
    if (word == "v") {
      ParseVertex(ss);
      return;
    } else if (word == "f") {
      ParseFace(ss);
      return;
    } else if (word == "vt") {
      ParseUVCoordinate(ss);
      return;
    } else
      throw UnknownCommandException("Unknown command!");
  }
}

void ObjGeometryImporter::ParseVertex(std::stringstream& vertex_params) {
  std::array<float, kDimensions> read_value;
  if (vertex_counter_ == kMaxVertices)
    throw WorldLimitsExceededException("Number of maximum vertices exceeded!");
  for (size_t dim = 0; dim < kSpatialDimensions; dim++) {
    if (!::MoreWordsRemaining(vertex_params))
      throw MalformedParametersException(
          "Vertex contained fewer than four coordinates!");
    vertex_params >> read_value[dim];
  }
  if (::MoreWordsRemaining(vertex_params))
    vertex_params >> read_value[kW];
  else
    read_value[kW] = 1.0;
  if (::MoreWordsRemaining(vertex_params))
    throw MalformedParametersException(
        "Vertex contained more than four coordinates!");
  vertices_[vertex_counter_++] =
      Vector4(read_value[0], read_value[1], read_value[2], read_value[3]);
}

void ObjGeometryImporter::ParseFace(std::stringstream& face_params) {
  std::array<size_t, kVerticesPerTriangle> vertex_indices;
  std::array<size_t, kVerticesPerTriangle> uv_indices = {0};
  if (triangle_counter_ == kMaxTriangles)
    throw WorldLimitsExceededException("Number of maximum triangles exceeded!");
  for (size_t i = 0; i < kVerticesPerTriangle; i++) {
    if (::MoreWordsRemaining(face_params)) {
      face_params >> vertex_indices[i];
      if (vertex_indices[i] == 0 || vertex_indices[i] > vertex_counter_)
        throw MalformedParametersException(
            "Face contained an invalid face index!");
      if (face_params.get() == '/') {
        face_params >> uv_indices[i];
        if (uv_indices[i] == 0 || uv_indices[i] > uv_counter_)
          throw MalformedParametersException(
              "Face contained an invalid UV index!");
      }
    } else
      throw UnsupportedPrimitiveException("Only triangle faces are supported!");
  }
  if (::MoreWordsRemaining(face_params))
    throw UnsupportedPrimitiveException("Only triangle faces are supported!");

  // OBJ uses CCW winding by default

  ::RearrangeIndicesByWindingDirection(vertex_indices.data());
  ::RearrangeIndicesByWindingDirection(uv_indices.data());
  std::array<Vertex, kVerticesPerTriangle> uv_vertices;
  for (size_t i = 0; i < kVerticesPerTriangle; i++) {
    if (uv_indices[i])
      uv_vertices[i] = Vertex(vertices_[vertex_indices[i] - 1],
                              uv_coordinates_[uv_indices[i] - 1]);
    else
      uv_vertices[i] = vertices_[vertex_indices[i] - 1];
  }
  TriangleSharedPointer triangle = std::make_shared<Triangle>(
      uv_vertices[0], uv_vertices[1], uv_vertices[2]);
  space_.EnqueueAddTriangle(triangle);
  triangle_counter_++;
}

void ObjGeometryImporter::ParseUVCoordinate(std::stringstream& uv_params) {
  std::array<float, kVerticesPerTriangle> read_value;
  if (uv_counter_ == kMaxVertices)
    throw WorldLimitsExceededException("Number of UV coordinates exceeded!");
  for (size_t dim = 0; dim < kUVDimensions; dim++) {
    uv_params >> std::ws;
    if (uv_params.eof())
      throw MalformedParametersException(
          "UV coordinates contained fewer than two coordinates!");
    uv_params >> read_value[dim];
  }
  uv_params >> std::ws;
  if (!uv_params.eof())
    throw MalformedParametersException(
        "UV coordinates contained more than two coordinates!");
  uv_coordinates_[uv_counter_++] = UVCoordinate(read_value[0], read_value[1]);
}