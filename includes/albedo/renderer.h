#pragma once

#include <vector>

#include <albedo/wgpu.h>

#include <albedo/components/camera.h>

#include <albedo/backend/buffer.h>
#include <albedo/backend/bind-group.h>
#include <albedo/backend/pipeline.h>
#include <albedo/backend/texture.h>
#include <albedo/scene.h>

namespace albedo
{

// TODO: make those structs inaccessible.

// TODO: padding can be removed from the CPU as the buffer is copied only
// from the byte 0 to the byte offset of the `height` field.
struct RenderInfo {
  uint width;
  uint height;
  uint instanceCount; // TODO: move to scene uniform
  uint lightCount; // TODO: move to scene uniform
  uint frameCount;
};

struct Uniforms {
  float time;
};

// @todo: replace by matrix4
struct CameraUniforms {
  glm::vec3 origin;
  float vFOV;
  glm::vec3 up;
  float padding_0;
  glm::vec3 right;
  float padding_1;
};

// TODO: add PIML? is it worth for people to access internal WGPU wrapper?
class Renderer
{

  public:

    Renderer() noexcept;

    ~Renderer() noexcept;

  public:

    // TODO: for now, the scene BVH is assumed to be completly built at this
    // stage.
    //
    // TODO: `init` should not automatically build and send the BVH to the GPU.
    // Could be a good idea to make the user explicitly do that and to give
    // him more control.
    Renderer&
    init(WGPUDeviceId deviceId, WGPUSurfaceId surfaceId);

    Renderer&
    buildTLAS(const Scene& scene);

    Renderer&
    resize(uint32_t width, uint32_t height);

    Renderer&
    startFrame(float deltaTime);

    Renderer&
    endFrame();

  public:

    Renderer&
    setCameraInfo(
      const components::PerspectiveCamera&,
      const glm::vec3& origin,
      const glm::vec3& up,
      const glm::vec3& right
    );

    Renderer&
    setProbe(float *data, uint width, uint height, uint comp = 3);

    inline uint32_t
    getWidth() { return m_swapChainDescriptor.width; }

    inline uint32_t
    getHeight() { return m_swapChainDescriptor.height; }

  private:

    RenderInfo m_info;
    Uniforms m_uniforms;
    CameraUniforms m_cameraUniforms;

    WGPUDeviceId m_deviceId;
    WGPUSurfaceId m_surfaceId;

    // TODO: move swap chain out of renderer?
    WGPUSwapChainDescriptor m_swapChainDescriptor;
    WGPUSwapChainId m_swapChainId;

    backend::ComputePipeline m_pathtracingPipeline;
    backend::BindGroup m_pathtracingBindGroup;
    // TODO: remove when migrating to a compute pass storing the result of
    // the light sampling in the Ray structure.
    backend::BindGroup m_pathtracingBindGroup2;
    backend::BindGroup m_cameraBindGroup;

    backend::RenderPipeline m_renderPipeline;
    backend::BindGroup m_blittingBindGroup;

    backend::Texture m_renderTarget;
    // TODO: remove when migrating to a compute pass storing the result of
    // the light sampling in the Ray structure.
    backend::Texture m_renderTarget2;
    backend::TextureSampler m_rtSampler;

    backend::Texture m_probe;
    backend::TextureSampler m_probeSampler;

    backend::Texture m_textures;

    WGPUCommandEncoderId m_commandEncoder; // TODO: refactor out?

    backend::Buffer<RenderInfo> m_renderInfoBuffer;
    backend::Buffer<BVHNodeGPU> m_nodesBuffer;
    backend::Buffer<InstanceGPU> m_instanceBuffer;
    backend::Buffer<Vertex> m_vertexBuffer;
    backend::Buffer<Mesh::IndexType> m_indicesBuffer;
    backend::Buffer<components::Material> m_materialBuffer;
    backend::Buffer<LightGPU> m_lightsBuffer;
    backend::Buffer<Uniforms> m_uniformsBuffer;
    backend::Buffer<CameraUniforms> m_cameraUniformsBuffer;

};

} // namespace filament
