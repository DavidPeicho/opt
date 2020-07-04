cmake_minimum_required(VERSION 3.13)

target_sources(webtracer PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/transform.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/material.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/light.cpp"
)
