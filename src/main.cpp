#include <iostream>

#include "geometry/space.hpp"
#include "utility/geometry_importer.hpp"

int main() {
  std::cout << "Hello, this is Software Renderer.\n\n";
  Space space;
  ObjGeomteryImporter obj_importer;
  obj_importer.ImportGeometryFromFile("assets/teapot.obj", space);
  return 0;
}