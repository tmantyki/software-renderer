#include <fstream>
#include <iostream>

#include "geometry/common.hpp"
#include "geometry/vertex.hpp"
#include "utility/geometry_importer.hpp"

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
    : space_(space), triangle_counter_(0), vertex_counter_(0) {}

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
    } else
      throw UnknownCommandException("Unknown command!");
  }
}

void ObjGeometryImporter::ParseVertex(std::stringstream& vertex_params) {
  std::array<float, kDimensions> read_value;
  if (vertex_counter_ == kMaxVertices)
    throw WorldLimitsExceededException("Number of maximum vertices exceeded!");
  for (size_t dim = 0; dim < kDimensions; dim++) {
    vertex_params >> std::ws;
    if (!vertex_params.eof()) {
      vertex_params >> read_value[dim];
    } else {
      assert(dim == 3);
      read_value[3] = 1.0;
    }
  }
  vertex_params >> std::ws;
  if (!vertex_params.eof())
    throw MalformedParametersException(
        "Vertex cointained more than four coordainates!");
  vertices_[vertex_counter_++] =
      Vector4(read_value[0], read_value[1], read_value[2], read_value[3]);
}

void ObjGeometryImporter::ParseFace(std::stringstream& face_params) {
  std::array<size_t, kVerticesPerTriangle> read_index;
  if (triangle_counter_ == kMaxTriangles)
    throw WorldLimitsExceededException("Number of maximum triangles exceeded!");
  for (size_t vertex_i = 0; vertex_i < kVerticesPerTriangle; vertex_i++) {
    face_params >> std::ws;
    if (!face_params.eof()) {
      face_params >> read_index[vertex_i];
      assert(read_index[vertex_i] > 0);
      assert(read_index[vertex_i] <= vertex_counter_);
    } else if (vertex_i != 3)
      throw UnsupportedPrimitiveException("Only triangle faces are supported!");
  }
  // OBJ uses CCW winding by default
  Vertex& vertex_1 = vertices_[read_index[0] - 1];
  Vertex& vertex_2 = kClockwiseWinding ? vertices_[read_index[2] - 1]
                                       : vertices_[read_index[1] - 1];
  Vertex& vertex_3 = kClockwiseWinding ? vertices_[read_index[1] - 1]
                                       : vertices_[read_index[2] - 1];
  TriangleSharedPointer triangle =
      std::make_shared<Triangle>(vertex_1, vertex_2, vertex_3);
  space_.EnqueueAddTriangle(triangle);
  triangle_counter_++;
}