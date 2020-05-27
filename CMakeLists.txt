cmake_minimum_required(VERSION 3.13)

project(webtracer)

set(CMAKE_BUILD_TYPE Debug)
set(WGPU_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/wgpu-native/target/debug/")
set(WGPU_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/wgpu-native/ffi/")
set(GLFW3_INCLUDE_DIR $ENV{GLFW3_INCLUDE_DIR})


set(TARGET_NAME webtracer)

add_executable(webtracer src/main.cpp)

set_target_properties(webtracer PROPERTIES CXX_STANDARD 17)

if(MSVC)
    add_definitions(-DWGPU_TARGET=WGPU_TARGET_WINDOWS)
    target_compile_options(${TARGET_NAME} PRIVATE /W4 /std:c++latest)
    set(OS_LIBRARIES "userenv" "ws2_32" "Dwmapi" "dbghelp" "d3dcompiler" "D3D12" "D3D11" "DXGI" "setupapi")
elseif(APPLE)
    add_definitions(-DWGPU_TARGET=WGPU_TARGET_MACOS)
    set(OS_LIBRARIES "-framework Cocoa" "-framework CoreVideo" "-framework IOKit" "-framework QuartzCore")
    target_compile_options(${TARGET_NAME} PRIVATE -g -fsanitize=address -fno-omit-frame-pointer -std=c++17 -x objective-c++)
else(MSVC)
if(USE_WAYLAND)
    add_definitions(-DWGPU_TARGET=WGPU_TARGET_LINUX_WAYLAND)
else(USE_WAYLAND)
    add_definitions(-DWGPU_TARGET=WGPU_TARGET_LINUX_X11)
endif(USE_WAYLAND)
    target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -pedantic)
endif(MSVC)

find_library(GLFW_LIBRARY glfw
  HINTS "${GLFW3_INSTALL_DIR}"
)

if(NOT GLFW_LIBRARY)
  message(FATAL_ERROR "glfw library not found")
endif()

find_library(WGPU_LIBRARY wgpu_native
    HINTS "${WGPU_INSTALL_DIR}"
)
if(NOT WGPU_LIBRARY)
  message(FATAL_ERROR "WebGPU library not found")
endif()

add_subdirectory(src)

target_include_directories(${TARGET_NAME} PUBLIC ${GLFW3_INCLUDE_DIR})
target_include_directories(${TARGET_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/includes")
target_include_directories(${TARGET_NAME} PUBLIC "${WGPU_INCLUDE_DIR}")
target_link_libraries(${TARGET_NAME} ${GLFW_LIBRARY} ${WGPU_LIBRARY} ${OS_LIBRARIES} -fsanitize=address)
