cmake_minimum_required(VERSION 3.13)

target_sources(webtracer PRIVATE
  instance.cpp
  mesh.cpp
  scene.cpp
)

add_subdirectory(backend)
add_subdirectory(plugins)
