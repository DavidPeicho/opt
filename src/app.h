#pragma once

#include <functional>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <albedo/components/camera.h>
#include <albedo/controller.h>
#include <albedo/scene.h>
#include <albedo/renderer.h>

namespace albedo
{

class App
{
  public:

    App();

    ~App();

  public:

    void
    run() noexcept;

    void
    stop() noexcept;

    void
    destroy() noexcept;

  public:

    inline void
    setScene(Scene* scene) { m_scene = scene; }

  private:

    static void
    mouseCallback(GLFWwindow* window, double x, double y);

    static void
    adapterCallback(WGPUAdapterId received, void* userdata);

  private:

    struct MouseCoords { double x; double y; };

    GLFWwindow* m_window;
    int m_windowWidth;
    int m_windowHeight;
    MouseCoords m_mouseCoords;

    bool m_running;

    Scene* m_scene;
    Renderer m_renderer;
    components::PerspectiveCamera m_camera;
    FPSCameraController* m_controller;


};

} // namespace albedo
