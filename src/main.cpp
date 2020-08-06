#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include "glm/glm.hpp"

#define WGPU_TARGET_MACOS 1
#define WGPU_TARGET_LINUX_X11 2
#define WGPU_TARGET_WINDOWS 3
#define WGPU_TARGET_LINUX_WAYLAND 4

#if WGPU_TARGET == WGPU_TARGET_MACOS
    #include <Foundation/Foundation.h>
    #include <QuartzCore/CAMetalLayer.h>
#endif

#include <GLFW/glfw3.h>

#if WGPU_TARGET == WGPU_TARGET_MACOS
  #define GLFW_EXPOSE_NATIVE_COCOA
#elif WGPU_TARGET == WGPU_TARGET_LINUX_X11
  #define GLFW_EXPOSE_NATIVE_X11
#elif WGPU_TARGET == WGPU_TARGET_LINUX_WAYLAND
  #define GLFW_EXPOSE_NATIVE_WAYLAND
#elif WGPU_TARGET == WGPU_TARGET_WINDOWS
  #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

#include <albedo/wgpu.h>

#include <glm/glm.hpp>
#include <albedoloader/gltf-loader.h>
#include <albedo/components/light.h>
#include <albedo/scene.h>

#include "app.h"

int main()
{
  albedo::App app;

  std::cout << "Loading scene..." << std::endl;

  albedo::loader::GLTFLoader loader;
  // auto sceneOptional = loader.load("./scenes/cornell-box.glb");
  auto sceneOptional = loader.load("./scenes/cornell-box-transforms.glb");
  if (!sceneOptional)
  {
    std::cerr << "Failed to load scene" << std::endl;
    return 1;
  }

  std::cout << "Scene loaded!" << std::endl;
  app.setScene(&*sceneOptional);
  app.run();

  return 0;
}
