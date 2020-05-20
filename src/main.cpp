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

#include <albedoloader/gltf-loader.h>
#include <albedo/scene.h>

GLFWwindow* window = nullptr;

void
initialize(WGPUAdapterId adapterId, void*)
{
  WGPUDeviceDescriptor deviceDescriptor{
    .extensions = { .anisotropic_filtering = false },
    .limits = { .max_bind_groups = 1 }
  };

  WGPUDeviceId deviceId = wgpu_adapter_request_device(adapter, &deviceDescriptor);

  albedo::Renderer renderer(deviceId);

  albedo::loader::GLTFLoader loader;
  auto scene = loader.load(renderer, "../box.glb");

  if (!scene)
  {
    std::cerr << "Failed to load scene" << std::endl;
    return 1;
  }

  albedo::accel::GPUAccelerationStructure accelerationStructure;
  accelerationStructure.addMeshes(loader.meshes());
  accelerationStructure.build(scene.value());

  render();
}

void render(albedo::Renderer& renderer)
{
  int width = 0;
  int height = 0;
  while (!glfwWindowShouldClose(window))
  {
    glfwGetWindowSize(window, &width, &height);
    // Resize if needed.
    renderer.setSize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
  }
}

int main() {
  if (!glfwInit()) {
      printf("Cannot initialize glfw");
      return 1;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window = glfwCreateWindow(640, 480, "wgpu with glfw", NULL, NULL);

  if (!window)
  {
    printf("Failed to create window!");
    return 1;
  }

  id metalLayer = [CAMetalLayer layer];
  NSWindow* nsWindow = glfwGetCocoaWindow(window);
  [nsWindow.contentView setWantsLayer : YES] ;
  [nsWindow.contentView setLayer : metalLayer] ;
  WGPUSurfaceId surface; = wgpu_create_surface_from_metal_layer(metalLayer);

  WGPUAdapterId adapterId = 0;
  WGPURequestAdapterOptions adapterOptions {
    .power_preference = WGPUPowerPreference_LowPower,
    .compatible_surface = surface
  };

  wgpu_request_adapter_async(&adapterOptions, 2 | 4 | 8,
      request_adapter_callback,
      initialize
  );

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
