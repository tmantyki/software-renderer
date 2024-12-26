#include <gtest/gtest.h>

#include "utility/geometry_importer.hpp"

class ObjGeometryImporterTest : public testing::Test {
 protected:
  ObjGeometryImporterTest() : obj_importer_(space_) {}

  void LoadObjFromInputStream(std::istream& input_stream) {
    obj_importer_.ImportGeometryFromInputStream(input_stream);
  }

  Space space_;
  ObjGeometryImporter obj_importer_;
  std::stringstream ss_;
};

TEST_F(ObjGeometryImporterTest, SimpleValidObjFile) {
  ss_ << "# Simple valid OBJ with comments, vertices,\n"
      << "# triangles and empty lines.\n"
      << "\n"
      << "v 0.0 1.1 2.2\n"
      << "v 3.3 4.4 5.5 2.0\n"
      << "v 6.6 7.7 8.8\n"
      << "\n"
      << "f 1 2 3\n"
      << "f 3 2 1\n"
      << "f 1 3 1\n";
  EXPECT_NO_THROW(LoadObjFromInputStream(ss_));
}

TEST_F(ObjGeometryImporterTest, NontriangularFacesAreNotSupported) {
  ss_ << "# This OBJ file contains a quad which, despite being supported by"
         "# the file format, is not supported by this engine.\n"
      << "\n"
      << "v 0.0 1.1 2.2 1.1 2.2\n"
      << "v 3.3 4.4 5.5\n"
      << "v 6.6 7.7 8.8\n"
      << "v 9.1 9.2 9.3\n"
      << "\n"
      << "f 1 2 3 4\n";
  EXPECT_THROW(LoadObjFromInputStream(ss_), MalformedParametersException);
}

TEST_F(ObjGeometryImporterTest, UnknownCommand) {
  ss_ << "# This invalid OBJ file contains an unsupported command 'xyz'.\n"
      << "\n"
      << "xyz\n";
  EXPECT_THROW(LoadObjFromInputStream(ss_), UnknownCommandException);
}

TEST_F(ObjGeometryImporterTest, MaximumNumberOfVerticesAndTriangles) {
  ss_ << "# This valid OBJ file includes the maximum allowed number of\n"
      << "# vertices and triangles\n\n";
  for (size_t i = 0; i < kMaxVertices; i++)
    ss_ << "v 0.0 1.1 2.2\n";
  for (size_t i = 0; i < kMaxTriangles; i++)
    ss_ << "f 1 2 3\n";
  EXPECT_NO_THROW(LoadObjFromInputStream(ss_));
}

TEST_F(ObjGeometryImporterTest, MaximumNumberOfVerticesExceeded) {
  ss_ << "# This invalid OBJ file exceeds the maximum allowed number of\n"
      << "# vertices\n\n";
  for (size_t i = 0; i < kMaxVertices + 1; i++)
    ss_ << "v 0.0 1.1 2.2\n";
  EXPECT_THROW(LoadObjFromInputStream(ss_), WorldLimitsExceededException);
}

TEST_F(ObjGeometryImporterTest, MaximumNumberOfTrianglesExceeded) {
  ss_ << "# This invalid OBJ file exceeds the maximum allowed number of\n"
      << "# triangles\n"
      << "\n"
      << "v 0.0 1.1 2.2\n"
      << "v 3.3 4.4 5.5\n"
      << "v 6.6 7.7 8.8\n";
  for (size_t i = 0; i < kMaxTriangles + 1; i++)
    ss_ << "f 1 2 3\n";
  EXPECT_THROW(LoadObjFromInputStream(ss_), WorldLimitsExceededException);
}
