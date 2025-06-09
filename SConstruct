env = Environment()
# env["CXX"] = "clang++" #TODO: add build param for compiler selection
env["CXX"] = "g++"
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
    "SDL2",
    "SDL2_image",
]
application_sources = [
    "main.cpp",
    "server/game_state.cpp",
    "ui/controller.cpp",
    "ui/rasterizer.cpp",
    "ui/ui.cpp",
]
geometry_sources = [
    "geometry/camera.cpp",
    "geometry/plane.cpp",
    "geometry/space.cpp",
    "geometry/texture.cpp",
    "geometry/transform.cpp",
    "utility/geometry_importer.cpp",
]
application_sources += geometry_sources
unit_test_sources = (
    geometry_sources
    + [
        "tests/geometry_importer_test.cpp",
        "tests/unit_test.cpp",
    ]
)
env_debug = env.Clone()
env_debug.AppendUnique(
    CXXFLAGS=[
        "-g",
        "-Og",
    ]
)
env_debug.AppendUnique(
    LINKFLAGS=[
        "-flto=auto",
    ]
)
env_release = env.Clone()
env_release.AppendUnique(
    CXXFLAGS=[
        "-g",
        "-O3",
        "-fno-rtti",
        "-flto=auto",
        "-march=haswell",
    ]
)
env_release.AppendUnique(
    CPPDEFINES=[
        "NDEBUG",
    ]
)
env_release.AppendUnique(
    LINKFLAGS=[
        "-flto=auto",
        "-fopt-info-vec=vectorization.log"
    ]
)

VariantDir("build/Debug/src", "src", duplicate=0)
debug_application_sources = ["build/Debug/src/" + s for s in application_sources]
debug_unit_test_sources = ["build/Debug/src/" + s for s in unit_test_sources]
env_debug.Program("build/Debug/renderer", debug_application_sources)
env_debug.Program("build/Debug/unit_test", debug_unit_test_sources)

VariantDir("build/Release/src", "src", duplicate=0)
release_application_sources = ["build/Release/src/" + s for s in application_sources]
release_unit_test_sources = ["build/Release/src/" + s for s in unit_test_sources]
env_release.Program("build/Release/renderer", release_application_sources)
env_release.Program("build/Release/unit_test", release_unit_test_sources)
