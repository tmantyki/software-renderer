#include <fstream>
#include <iostream>

#include "geometry/common.hpp"
#include "geometry/vertex.hpp"
#include "utility/geometry_importer.hpp"

GeometryImporter::GeometryImporter(Space& space) : space_(space) {}

ObjGeometryImporter::ObjGeometryImporter(Space& space)
    : GeometryImporter(space), triangle_counter_(0), vertex_counter_(0) {}

bool ObjGeometryImporter::ImportGeometryFromFile(const char* filename) {
  std::ifstream input_file;
  std::string line;
  input_file.open(filename);
  if (!input_file.is_open()) {
    std::cout << "Error: could not open file '" << filename << "'";
    return false;
  }
  while (std::getline(input_file, line)) {
    if (!ParseLine(line)) {
      input_file.close();
      return false;
    }
  }
  input_file.close();
  space_.UpdateSpace();
  std::cout << "Successfully read " << vertex_counter_ << " vertices and "
            << triangle_counter_ << " triangles from " << filename << ".\n";
  return true;
}

bool ObjGeometryImporter::ParseLine(const std::string& line) {
  std::stringstream ss(line);
  ss >> std::ws;
  std::string word;
  if (std::getline(ss, word, ' ')) {
    if (word == "v") {
      return ParseVertex(ss);
    } else if (word == "f") {
      return ParseFace(ss);
    } else {
      std::cout << "Error: unknown command '" << word << "'\n";
      return false;
    }
  }
  return true;  // empty lines return true
}

bool ObjGeometryImporter::ParseVertex(std::stringstream& vertex_params) {
  std::array<float, kDimensions> read_value;
  if (vertex_counter_ == kMaxVertices) {
    std::cout << "Error: number of vertices exceeds " << kMaxVertices << ".\n";
    return false;
  }
  for (size_t dim = 0; dim < kDimensions; dim++) {
    vertex_params >> std::ws;
    if (!vertex_params.eof()) {
      vertex_params >> read_value[dim];
    } else {
      assert(dim == 3);  // #TODO better error handling
      read_value[3] = 1.0;
    }
  }
  vertex_params >> std::ws;
  if (!vertex_params.eof()) {
    std::cout << "Error: vertex contained more than four coordinates.\n";
    return false;
  }
  vertices_[vertex_counter_++] =
      Vector4(read_value[0], read_value[1], read_value[2], read_value[3]);
  return true;
}

bool ObjGeometryImporter::ParseFace(std::stringstream& face_params) {
  std::array<size_t, kVerticesPerTriangle> read_index;
  if (triangle_counter_ == kMaxTriangles) {
    std::cout << "Error: number of triangles exceeds " << kMaxTriangles
              << ".\n";
    return false;
  }
  for (size_t vertex_i = 0; vertex_i < kVerticesPerTriangle; vertex_i++) {
    face_params >> std::ws;
    if (!face_params.eof()) {
      face_params >> read_index[vertex_i];
      assert(read_index[vertex_i] > 0);
      assert(read_index[vertex_i] <= vertex_counter_);
    } else if (vertex_i != 3) {
      std::cout << "Error: only triangle faces are supported.\n";
      return false;
    }
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
  return true;
}