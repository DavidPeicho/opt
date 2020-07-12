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
#include <albedo/debug/scene-debugger.h>
#include <albedo/controller.h>
#include <albedo/components/light.h>
#include <albedo/scene.h>
#include <albedo/renderer.h>

struct App
{
  glm::vec2 mouse;
  glm::vec2 mouseDelta;
};

GLFWwindow* window = nullptr;
WGPUSurfaceId gSurfaceId = 0;
App gAppState;

void request_adapter_callback(WGPUAdapterId received, void *userdata)
{
  *(WGPUAdapterId*)userdata = received;
}

void mouseCb(GLFWwindow* window, double xpos, double ypos)
{
  auto mouse = glm::vec2(xpos, ypos);
  gAppState.mouseDelta = mouse - gAppState.mouse;
  gAppState.mouse = std::move(mouse);

  // TODO: remove the size getter. Normalization can be done at anther place.
  int width = 1;
  int height = 1;
  glfwGetWindowSize(window, &width, &height);

  gAppState.mouseDelta.x /= (float)width;
  gAppState.mouseDelta.y /= (float)height;

  std::cout << glm::to_string(gAppState.mouseDelta) << std::endl;
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

  // Startup mouse position to avoid getting mouse jump.
  double x = 0.0;
  double y = 0.0;
  glfwGetCursorPos(window, &x, &y);
  gAppState.mouse = glm::vec2(x, y);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouseCb);

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

  // DEBUG
  albedo::Entity lightEntity;
  albedo::components::Light l{};
  l.intensity = 20.0;
  l.width = 3.0;
  l.height = 2.0;

  scene.lights().createComponent(lightEntity, std::move(l));

  scene.transforms().createComponent(lightEntity);
  auto lightTransform = scene.transforms().getComponent(lightEntity);
  lightTransform->rotateGlobalX(glm::pi<float>() * 0.5);
  lightTransform->translateGlobalY(3.5);
  // END DEBUG

  std::cout << "Building scene...." << std::endl;
  scene.build();
  std::cout << "Scene built!" << std::endl;

  std::cout << "Updating scene...." << std::endl;
  scene.update();
  std::cout << "Scene updated!" << std::endl;

  albedo::Renderer renderer(deviceId, gSurfaceId);
  renderer.init(scene);

  albedo::components::PerspectiveCamera camera;
  albedo::FPSCameraController controller(glm::vec3(0.0, 0.0, 10.0));
  renderer.setCameraInfo(
        camera,
        controller.getOrigin(),
        controller.getUp(),
        controller.getRight()
  );

  int width = 0;
  int height = 0;
  float lastTime = glfwGetTime();
  float delta = 0.0;

  float speed = 0.5;
  bool dirty = false;

  while (!glfwWindowShouldClose(window))
  {
    float time = glfwGetTime();
    delta = time - lastTime;

    // Inputs.
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    { controller.forward(speed); }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    { controller.backward(speed); }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    { controller.left(speed); }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    { controller.right(speed); }

    controller.rotate(gAppState.mouseDelta);
    controller.update(delta);

    glfwGetWindowSize(window, &width, &height);

    // std::cout << glm::to_string(controller.getOrigin())
    // << " " << glm::to_string(controller.getUp())
    // << " " << glm::to_string(controller.getRight())
    // << std::endl;

    // Resize if needed.
    renderer.resize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

    if (controller.isDirty())
    {
      renderer.setCameraInfo(
        camera,
        controller.getOrigin(),
        controller.getUp(),
        controller.getRight()
      );
    }

    renderer.startFrame(delta);
    renderer.endFrame();

    glfwPollEvents();

    lastTime = time;
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
