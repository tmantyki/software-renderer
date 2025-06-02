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
    "server/player.cpp",
    "ui/controller.cpp",
    "ui/rasterizer.cpp",
    "ui/ui.cpp",
    "utility/timer.cpp",
]
geometry_sources = [
    "geometry/camera.cpp",
    "geometry/coordinate.cpp",
    "geometry/direction.cpp",
    "geometry/line_segment.cpp",
    "geometry/plane.cpp",
    "geometry/point.cpp",
    "geometry/space.cpp",
    "geometry/texture.cpp",
    "geometry/transform.cpp",
    "geometry/triangle.cpp",
    "geometry/uv_coordinate.cpp",
    "geometry/vertex.cpp",
    "utility/geometry_importer.cpp",
]
utility_sources = [
    "utility/timer.cpp",
]
application_sources += geometry_sources
unit_test_sources = (
    geometry_sources
    + utility_sources
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
        # "-fsanitize=address,undefined",
        "-flto=auto",
    ]
)
env_release = env.Clone()
env_release.AppendUnique(
    CXXFLAGS=[
        "-O3",
        "-g",
        "-flto=auto",
    ]
)
env_release["CPPDEFINES"] = [
    "NDEBUG",
]

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
