env = Environment()
env["CXXFLAGS"] = [
  "-std=c++17",
  "-Wall",
  "-Wextra",
  "-Wpedantic",
  "-Werror",
]
env["LINKFLAGS"] = [
  "-fsanitize=address,undefined",
]
env["CPPPATH"] = [
  "/usr/include/eigen3",
]
VariantDir("build", "src", duplicate=0)
sources = [
  "build/triangle.cpp",
  ]
env["LIBS"] = [
  "gtest",
  "gtest_main",
]
env.Program("renderer", sources + ["build/main.cpp"])
env.Program("unit_test", sources + ["build/unit_test.cpp"])
