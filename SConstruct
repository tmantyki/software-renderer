env = Environment()
env["CXXFLAGS"] = [
    "-std=c++17",
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-Werror",
]
env["CPPPATH"] = [
    "src",
    "/usr/include/eigen3",
]
env["LIBS"] = [
    "gtest",
    "gtest_main",
]
sources = [
    "geometry/coordinate.cpp",
    "geometry/point.cpp",
    "geometry/direction.cpp",
    "geometry/line_segment.cpp",
    "geometry/plane.cpp",
    "geometry/vertex.cpp",
    "geometry/triangle.cpp",
    "geometry/space.cpp",
    "geometry/transform.cpp",
    "geometry/camera.cpp",
    "utility/timer.cpp",
]
env_debug = env.Clone()
env_debug.AppendUnique(
    CXXFLAGS=[
        "-g",
        "-Og",
    ]
)
env_debug["LINKFLAGS"] = [
    "-fsanitize=address,undefined",
]
env_release = env.Clone()
env_release.AppendUnique(
    CXXFLAGS=[
        "-O2",
    ]
)
env_release["CPPDEFINES"] = ["NDEBUG"]
VariantDir("build/Debug/src", "src", duplicate=0)
debug_sources = ["build/Debug/src/" + s for s in sources]
env_debug.Program("build/Debug/renderer", debug_sources + ["build/Debug/src/main.cpp"])
env_debug.Program(
    "build/Debug/unit_test", debug_sources + ["build/Debug/src/tests/unit_test.cpp"]
)
VariantDir("build/Release/src", "src", duplicate=0)
release_sources = ["build/Release/src/" + s for s in sources]
env_release.Program(
    "build/Release/renderer", release_sources + ["build/Release/src/main.cpp"]
)
env_release.Program(
    "build/Release/unit_test",
    release_sources + ["build/Release/src/tests/unit_test.cpp"],
)
