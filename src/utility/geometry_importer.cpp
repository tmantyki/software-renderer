#include <fstream>
#include <iostream>

#include "geometry/common.hpp"
#include "geometry/vertex.hpp"
#include "utility/geometry_importer.hpp"

namespace {}

GeometryImporter::GeometryImporter(Space& space) : space_(space) {}

ObjGeometryImporter::ObjGeometryImporter(Space& space)
    : GeometryImporter(space) {}

void ObjGeometryImporter::ImportGeometryFromFile(const char* filename) {
  std::cout << filename << "\n";
  std::ifstream input_file;
  input_file.open(filename);
  if (!input_file.is_open()) {
    std::cout << "Error: could not open file " << filename << "\n";
    return;
  }
  for (size_t l : {0, 1, 2}) {
    std::string line;
    std::getline(input_file, line);
    std::cout << l + 1 << ": " << line << "\n";
    std::stringstream ss(line);
    std::getline(ss, line, ' ');
    std::cout << "sub: " << line << "\n";
  }
  input_file.close();
}