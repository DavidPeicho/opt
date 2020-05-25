#include <albedo/wgpu.h>

namespace albedo
{

namespace backend
{

// TODO: move to sources and use PIMPL.
class RenderPipeline
{
  public:

    RenderPipeline() noexcept;

    ~RenderPipeline() noexcept;

  public:

    void
    create(WGPUDeviceId deviceId, WGPUPipelineLayoutId pipelineLayoutId);

    // TODO: store a string as the entry?
    void
    bindVertexShader(WGPUShaderModuleId module, const char* entry);

    // TODO: store a string as the entry?
    void
    bindFragmentShader(WGPUShaderModuleId module, const char* entry);

    inline WGPURenderPipelineId
    id() const { return m_id; }

  private:

    WGPURenderPipelineId m_id;

    WGPURenderPipelineDescriptor m_descriptor;
    WGPUProgrammableStageDescriptor m_fragmentStageDescriptor;
    WGPURasterizationStateDescriptor m_rasterizationStateDescriptor;
    WGPUColorStateDescriptor m_colorStateDescriptor;

};

} // namespace backend

} // namespace albedo
