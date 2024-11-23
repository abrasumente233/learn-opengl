project(
  'learn-opengl',
  ['c', 'cpp'],
  default_options: [
    'warning_level=3',
    'cpp_std=c++17',
    'wrap_mode=forcefallback'
  ]
)

# assimp

cmake = import('cmake')
assimp_opt = cmake.subproject_options()
assimp_opt.add_cmake_defines({
  'BUILD_SHARED_LIBS': false,
  'CMAKE_BUILD_TYPE': get_option('buildtype') == 'debug' ? 'Debug' : 'Release',
  'ASSIMP_BUILD_ASSIMP_TOOLS': false,
  'ASSIMP_BUILD_TESTS': false,
  'ASSIMP_BUILD_ZLIB': false,

  'ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT': false,
  'ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT': false,

  'ASSIMP_BUILD_GLB_IMPORTER': true,
  'ASSIMP_BUILD_OBJ_IMPORTER': true,
  'ASSIMP_BUILD_GLTF_IMPORTER': true,
})
assimp_dep = cmake.subproject('assimp', options: assimp_opt).dependency('assimp')

glfw = dependency('glfw3')
glm = dependency('glm')
imgui = dependency('imgui')

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
  dependencies: [glfw, glm, imgui, assimp_dep],
)
