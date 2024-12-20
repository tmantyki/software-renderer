#include <fstream>
#include <iostream>

#include "geometry/common.hpp"
#include "geometry/vertex.hpp"
#include "utility/geometry_importer.hpp"

namespace {}

GeometryImporter::GeometryImporter(Space& space) : space_(space) {}

ObjGeometryImporter::ObjGeometryImporter(Space& space)
    : GeometryImporter(space), triangle_counter_(0), vertex_counter_(0) {}

bool ObjGeometryImporter::ImportGeometryFromFile(const char* filename) {
  std::ifstream input_file;
  std::string line;
  input_file.open(filename);
  if (!input_file.is_open()) {
    std::cout << "Error: could not open file " << filename << "\n";
    return false;
  }

  while (std::getline(input_file, line)) {
    if (!ParseLine(line)) {
      input_file.close();
      return false;
    }
  }

  input_file.close();
  return true;
}

bool ObjGeometryImporter::ParseLine(const std::string& line) {
  std::stringstream ss(line);
  std::string word;
  if (std::getline(ss, word, ' ')) {
    if (word == "v") {
      return ParseVertex(ss);
    } else {
      // std::cout << "Error! Unkown command: " << word << "\n";
      return false;
    }
    return true;
  }
  return false;
}

bool ObjGeometryImporter::ParseVertex(std::stringstream& vertex_params) {
  float read_value[4];
  if (vertex_counter_ == kMaxVertices) {
    std::cout << "Error: " << "number of vertices exceeds " << kMaxVertices
              << ".\n";
    return false;
  }
  for (size_t dim : {0, 1, 2, 3}) {
    vertex_params >> std::ws;
    if (vertex_params) {
      vertex_params >> read_value[dim];
    } else {
      assert(dim == 3);
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