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
env['LINKFLAGS'] = [
  '-fsanitize=address,undefined',
]
env['LIBS'] = [
  'gtest',
  'gtest_main',
]
sources = [
  'build/coordinate.cpp',
  'build/point.cpp',
  'build/direction.cpp',
  'build/line_segment.cpp',
  'build/plane.cpp',
]
VariantDir('build', 'src', duplicate=0)
env.Program('renderer', sources + ['build/main.cpp'])
env.Program('unit_test', sources + ['build/unit_test.cpp'])
