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
glm = dependency('glm')

inc = include_directories('./deps/glad/include', './deps/stb/include')
glad = library(
  'glad',
  './deps/glad/glad.c',
  include_directories: inc
)

executable(
  'main',
  'src/main.cpp',
  include_directories: inc,
  link_with: glad,
  dependencies: [glfw, glm],
)
