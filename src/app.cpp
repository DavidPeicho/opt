#define WGPU_TARGET_MACOS 1
#define WGPU_TARGET_LINUX_X11 2
#define WGPU_TARGET_WINDOWS 3
#define WGPU_TARGET_LINUX_WAYLAND 4

#if WGPU_TARGET == WGPU_TARGET_MACOS
  #include <Foundation/Foundation.h>
  #include <QuartzCore/CAMetalLayer.h>
#endif

#if WGPU_TARGET == WGPU_TARGET_MACOS
  #define GLFW_EXPOSE_NATIVE_COCOA
#elif WGPU_TARGET == WGPU_TARGET_LINUX_X11
  #define GLFW_EXPOSE_NATIVE_X11
#elif WGPU_TARGET == WGPU_TARGET_LINUX_WAYLAND
  #define GLFW_EXPOSE_NATIVE_WAYLAND
#elif WGPU_TARGET == WGPU_TARGET_WINDOWS
  #define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include "app.h"
#include <albedo/wgpu.h>

// @todo: that's gross, fix that.
#define STB_IMAGE_IMPLEMENTATION
#include "plugins/includes/albedoloader/stb_image.h"

#include <glm/glm.hpp>

namespace albedo
{

void
App::mouseCallback(GLFWwindow* window, double x, double y)
{
  auto* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));

  auto delta = glm::vec2(
    glm::radians(x - app->m_mouseCoords.x),
    glm::radians(y - app->m_mouseCoords.y)
  );
  app->m_mouseCoords = { x, y };
  app->m_controller->rotate(delta);
}

void
App::adapterCallback(WGPUAdapterId received, void* userdata)
{
  *(WGPUAdapterId*)userdata = received;
}

App::App()
  : m_window(nullptr)
  , m_windowWidth(0)
  , m_windowHeight(0)
  , m_mouseCoords({ .x = 0.0, .y = 0.0 })
  , m_running(true)
{
  if (!glfwInit())
  {
    throw "failed to initialize GLFW";
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  m_window = glfwCreateWindow(640, 480, "OPT: Over-engineered PathTracer", NULL, NULL);

  if (!m_window)
  {
    throw "failed to create GLFW window";
  }

  glfwSetWindowUserPointer(m_window, this);

  // TODO: add #ifdef for othjer plaftorms
  id metalLayer = [CAMetalLayer layer];
  NSWindow* nsWindow = glfwGetCocoaWindow(m_window);
  [nsWindow.contentView setWantsLayer : YES] ;
  [nsWindow.contentView setLayer : metalLayer] ;

  // TODO: save surface ID to delete it.
  auto surfaceId = wgpu_create_surface_from_metal_layer(metalLayer);

  WGPUAdapterId adapterId = { 0 };
  auto adapterOptions = WGPURequestAdapterOptions {
    .power_preference = WGPUPowerPreference_HighPerformance,
    .compatible_surface = surfaceId
  };
  wgpu_request_adapter_async(
    &adapterOptions,
    2 | 4 | 8,
    false,
    App::adapterCallback,
    (void*)&adapterId
  );

  WGPUCLimits limits {
    .max_bind_groups = 2
  };

  WGPUDeviceId deviceId = wgpu_adapter_request_device(
    adapterId,
    0,
    &limits,
    true,
    NULL
  );

  glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(m_window, App::mouseCallback);

  m_renderer.init(adapterId, surfaceId);
  m_controller = new FPSCameraController(glm::vec3(0.0, 0.0, 10.0));

  std::cout << "Loading probe..." << std::endl;
  int imgWidth = 0;
  int imgHeight = 0;
  int imgComp = 0;
  float *data = stbi_loadf("./scenes/uffizi-large.hdr", &imgWidth, &imgHeight, &imgComp, 4);
  if (data && imgWidth > 0 && imgHeight > 0 && imgComp > 0)
  {
    std::cout << imgComp << std::endl;
    m_renderer.setProbe(
      data,
      static_cast<uint>(imgWidth),
      static_cast<uint>(imgHeight),
      static_cast<uint>(imgComp)
    );
  }
  else
  {
    std::cout << "probe loading failed!" << std::endl;
  }

}

App::~App() { destroy(); }

void
App::run() noexcept
{
  if (!m_window) { return; }

  float lastTime = glfwGetTime();
  float delta = 0.0;

  // DEBUG
  albedo::Entity lightEntity;
  albedo::components::Light l{};
  l.intensity = 2.5;
  l.width = 3.0;
  l.height = 2.0;
  m_scene->lights().createComponent(lightEntity, std::move(l));
  m_scene->transforms().createComponent(lightEntity);
  auto lightTransform = m_scene->transforms().getComponent(lightEntity);
  lightTransform->rotateGlobalX(glm::pi<float>() * 0.5);
  lightTransform->translateGlobalY(3.5);
  // END DEBUG

  std::cout << "Building scene..." << std::endl;
  m_scene->build();
  std::cout << "m_scene built!" << std::endl;
  std::cout << "Updating scene..." << std::endl;
  m_scene->update();
  std::cout << "scene updated!" << std::endl;

  m_renderer.buildTLAS(*m_scene);
  m_renderer.setCameraInfo(
    m_camera,
    m_controller->getOrigin(),
    m_controller->getUp(),
    m_controller->getRight()
  );

  // Startup mouse position to avoid getting mouse jump.
  glfwGetCursorPos(m_window, &m_mouseCoords.x, &m_mouseCoords.y);

  while (!glfwWindowShouldClose(m_window) && m_running)
  {
    float time = glfwGetTime();
    delta = time - lastTime;

    // Inputs.
    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
    { m_controller->forward(); }
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
    { m_controller->backward(); }
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
    { m_controller->left(); }
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
    { m_controller->right(); }

    m_controller->update(delta);
    if (m_controller->isDirty())
    {
      m_renderer.setCameraInfo(
        m_camera,
        m_controller->getOrigin(),
        m_controller->getUp(),
        m_controller->getRight()
      );
    }

    glfwGetWindowSize(m_window, &m_windowWidth, &m_windowHeight);
    // Resize if needed.
    m_renderer.resize(
      static_cast<uint32_t>(m_windowWidth),
      static_cast<uint32_t>(m_windowHeight)
    );

    m_renderer.startFrame(delta);
    m_renderer.endFrame();

    glfwPollEvents();

    lastTime = time;
  }
}

void
App::destroy() noexcept
{
  if (m_window != nullptr)
  {
    glfwDestroyWindow(m_window);
    glfwTerminate();
    delete m_controller;
  }
}

void
App::stop() noexcept { m_running = false; }

} // namespace albedo
