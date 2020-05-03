cmake_minimum_required(VERSION 3.13)

target_sources(webtracer PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/gltf-loader.cpp")
target_include_directories(webtracer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/includes")
