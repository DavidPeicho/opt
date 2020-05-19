#pragma once

#include <vector>

#include <albedo/wgpu.h>

namespace albedo
{

class Renderer
{

  public:

    Renderer(WGPUDeviceId deviceId);

  public:

    Renderer&
    init();

    Renderer&
    render();

    void
    destroy();

    template <typename T>
    Renderer&
    deleteResource(T resource);

  private:

    uint16_t m_renderWidth;
    uint16_t m_renderHeight;

    WGPUDeviceId m_deviceId;



};

} // namespace filament
