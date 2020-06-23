#pragma once

#include <initializer_list>
#include <memory>

#include <albedo/wgpu.h>
#include <albedo/backend/bind-group.h>
#include <albedo/backend/object.h>

namespace albedo
{

namespace backend
{

class PipelineLayout: public WGPUObject<WGPUPipelineLayoutDescriptor>
{
  public:

    PipelineLayout() noexcept = default;

    ~PipelineLayout() noexcept;

  public:

    void
    create(WGPUDeviceId deviceId, const std::initializer_list<WGPUBindGroupLayoutId>&);

};

// TODO: move to sources and use PIMPL.
class RenderPipeline: public WGPUObject<WGPURenderPipelineDescriptor>
{
  public:

    RenderPipeline() noexcept;

    ~RenderPipeline() noexcept;

  public:

    void
    create(WGPUDeviceId deviceId);

    // TODO: store a string as the entry?
    void
    bindVertexShader(WGPUShaderModuleId module, const char* entry);

    // TODO: store a string as the entry?
    void
    bindFragmentShader(WGPUShaderModuleId module, const char* entry);

    inline void
    setPipelineLayout(const std::shared_ptr<PipelineLayout>& layout)
    {
      m_layout = layout;
    }

  private:

    WGPUProgrammableStageDescriptor m_fragmentStageDescriptor;
    WGPURasterizationStateDescriptor m_rasterizationStateDescriptor;
    WGPUColorStateDescriptor m_colorStateDescriptor;

    std::shared_ptr<PipelineLayout> m_layout;

};

class ComputePipeline: public WGPUObject<WGPUComputePipelineDescriptor>
{
public:

    ComputePipeline() noexcept = default;

    ~ComputePipeline() noexcept;

  public:

    void
    create(WGPUDeviceId deviceId);

    void
    bindShader(WGPUShaderModuleId module, const char* entry);

    inline void
    setPipelineLayout(const std::shared_ptr<PipelineLayout>& layout)
    {
      m_layout = layout;
    }

  private:

    std::shared_ptr<PipelineLayout> m_layout;
};

} // namespace backend

} // namespace albedo
