cmake_minimum_required(VERSION 3.13)

target_sources(webtracer PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/texture.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/pipeline.cpp"
)
