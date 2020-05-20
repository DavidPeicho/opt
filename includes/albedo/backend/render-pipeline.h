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

    RenderPipeline() noexcept = default;

  public:

    void
    create(WGPUDeviceId deviceId);

    // TODO: store a string as the entry?
    void
    bindVertexShader(WGPUShaderModuleId module, const char* entry);

    // TODO: store a string as the entry?
    void
    bindFragmentShader(WGPUShaderModuleId module, const char* entry);

  private:

    WGPURenderPipelineId m_id;

    WGPURenderPipelineDescriptor m_descriptor;
    WGPUProgrammableStageDescriptor m_fragmentStageDescriptor;
    WGPUColorStateDescriptor m_colorStateDescriptor;

};

} // namespace backend

} // namespace albedo
