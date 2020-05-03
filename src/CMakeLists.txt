cmake_minimum_required(VERSION 3.13)

target_sources(webtracer PRIVATE
  instance.cpp
  scene.cpp
)

add_subdirectory(plugins)
