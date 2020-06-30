cmake_minimum_required(VERSION 3.13)

target_sources(webtracer PRIVATE
  mesh.cpp
  scene.cpp
  renderer.cpp
)

add_subdirectory(backend)
add_subdirectory(debug)
add_subdirectory(components)
add_subdirectory(plugins)
add_subdirectory(utils)
