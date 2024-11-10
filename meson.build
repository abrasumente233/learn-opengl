project(
  'learn-opengl',
  ['c', 'cpp'],
  default_options: [
    'warning_level=3',
    'cpp_std=c++17',
    'wrap_mode=forcefallback'
  ]
)


glfw = dependency('glfw3')

gladinc = include_directories('./deps/glad/include')
glad = library(
  'glad',
  './deps/glad/glad.c',
  include_directories: gladinc
)

executable(
  'main',
  'src/main.cpp',
  include_directories: gladinc,
  link_with: glad,
  dependencies: [glfw],
)
