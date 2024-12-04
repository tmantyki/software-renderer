env = Environment()
env['CXXFLAGS'] = [
  '-std=c++17',
  '-Wall',
  '-Wextra',
  '-Wpedantic',
  '-Werror',
]
env['CPPPATH'] = [
  '/usr/include/eigen3'
]

env['LIBS'] = [
  'gtest',
  'gtest_main',
]
sources = [
  'timer.cpp',
  'coordinate.cpp',
  'point.cpp',
  'direction.cpp',
  'line_segment.cpp',
  'plane.cpp',
  'vertex.cpp',
  'triangle.cpp',
  'space.cpp',
  'transform.cpp',
  'camera.cpp',
]
env_debug = env.Clone()
env_debug.AppendUnique(CXXFLAGS=[
  '-g',
  '-Og',
])
env_debug['LINKFLAGS'] = [
  '-fsanitize=address,undefined',
]
env_release = env.Clone()
env_release.AppendUnique(CXXFLAGS=[
  '-O2',
])
env_release['CPPDEFINES'] = ['NDEBUG']
VariantDir('build/Debug/src', 'src', duplicate=0)
debug_sources = ['build/Debug/src/' + s for s in sources]
env_debug.Program('build/Debug/renderer', debug_sources + ['build/Debug/src/main.cpp'])
env_debug.Program('build/Debug/unit_test', debug_sources + ['build/Debug/src/unit_test.cpp'])
VariantDir('build/Release/src', 'src', duplicate=0)
release_sources = ['build/Release/src/' + s for s in sources]
env_release.Program('build/Release/renderer', release_sources + ['build/Release/src/main.cpp'])
env_release.Program('build/Release/unit_test', release_sources + ['build/Release/src/unit_test.cpp'])
