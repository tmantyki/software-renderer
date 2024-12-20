#include <iostream>

#include "geometry/space.hpp"
#include "utility/geometry_importer.hpp"

int main() {
  std::cout << "Hello, this is Software Renderer.\n\n";
  Space space;
  ObjGeometryImporter obj_importer(space);
  obj_importer.ImportGeometryFromFile("assets/teapot.obj");
  return 0;
}