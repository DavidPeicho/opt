#pragma once

#include <vector>

#include <albedo/wgpu.h>

#include <albedo/backend/buffer.h>
#include <albedo/backend/render-pipeline.h>
#include <albedo/resource-manager.h>

namespace albedo
{

// TODO: add PIML? is it worth for people to access internal WGPU wrapper?
class Renderer
{

  public:

    Renderer(
      WGPUDeviceId deviceId,
      WGPUSurfaceId surfaceId,
      uint32_t width = 0,
      uint32_t height = 0
    );

    ~Renderer();

  public:

    Renderer&
    init();

    Renderer&
    resize(uint32_t width, uint32_t height);

    Renderer&
    startFrame();

    Renderer&
    endFrame();

  public:

    inline uint32_t
    getWidth() { return m_swapChainDescriptor.width; }

    inline uint32_t
    getHeight() { return m_swapChainDescriptor.height; }

  private:

    WGPUDeviceId m_deviceId;
    WGPUSurfaceId m_surfaceId;

    // TODO: move swap chain out of renderer?
    WGPUSwapChainDescriptor m_swapChainDescriptor;
    WGPUSwapChainId m_swapChainId;

    backend::RenderPipeline m_renderPipeline;

    backend::Buffer<BVHNodeGPU> m_nodesBuffer;
    backend::Buffer<Vertex> m_vertexBuffer;
    backend::Buffer<Mesh::IndexType> m_indicesBuffer;

};

} // namespace filament
