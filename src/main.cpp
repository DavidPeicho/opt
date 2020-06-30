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

#include <albedoloader/gltf-loader.h>
#include <albedo/debug/scene-debugger.h>
#include <albedo/scene.h>
#include <albedo/renderer.h>

GLFWwindow* window = nullptr;
WGPUSurfaceId gSurfaceId = 0;

void request_adapter_callback(WGPUAdapterId received, void *userdata) {
    *(WGPUAdapterId*)userdata = received;
}

void render(albedo::Renderer& renderer)
{
  int width = 0;
  int height = 0;

  float lastTime = glfwGetTime();
  float delta = 0.0;
  while (!glfwWindowShouldClose(window))
  {
    float time = glfwGetTime();
    delta = time - lastTime;

    glfwGetWindowSize(window, &width, &height);
    // Resize if needed.
    renderer.resize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    renderer.startFrame(delta);
    renderer.endFrame();
    glfwPollEvents();

    lastTime = time;
  }
}

int main() {
  if (!glfwInit()) {
      printf("Cannot initialize glfw");
      return 1;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window = glfwCreateWindow(640, 480, "OPT: Over-engineered PathTracer", NULL, NULL);

  if (!window)
  {
    printf("Failed to create window!");
    return 1;
  }

  id metalLayer = [CAMetalLayer layer];
  NSWindow* nsWindow = glfwGetCocoaWindow(window);
  [nsWindow.contentView setWantsLayer : YES] ;
  [nsWindow.contentView setLayer : metalLayer] ;
  gSurfaceId = wgpu_create_surface_from_metal_layer(metalLayer);

  WGPUAdapterId adapterId = { 0 };
  auto adapterOptions = WGPURequestAdapterOptions {
    .power_preference = WGPUPowerPreference_HighPerformance,
    .compatible_surface = gSurfaceId
  };
  wgpu_request_adapter_async(
    &adapterOptions,
    2 | 4 | 8,
    false,
    request_adapter_callback,
    (void*)&adapterId
  );

  WGPUCLimits limits {
    .max_bind_groups = 2
  };

  WGPUDeviceId deviceId = wgpu_adapter_request_device(adapterId, 0, &limits, NULL);

  std::cout << "Loading scene..." << std::endl;

  albedo::loader::GLTFLoader loader;
  auto sceneOptional = loader.load("./cornell-box.glb");
  if (!sceneOptional)
  {
    std::cerr << "Failed to load scene" << std::endl;
    return 1;
  }

  std::cout << "Scene loaded!" << std::endl;

  auto& scene = *sceneOptional;

  std::cout << "Building scene...." << std::endl;
  scene.build();
  std::cout << "Scene built!" << std::endl;

  albedo::debug::SceneDebugger::writeGraphiz(scene, 0, "debug.dot");
  return 0;

  std::cout << "Updating scene...." << std::endl;
  scene.update();
  std::cout << "Scene updated!" << std::endl;

  albedo::Renderer renderer(deviceId, gSurfaceId);
  renderer.init(scene);

  render(renderer);

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
