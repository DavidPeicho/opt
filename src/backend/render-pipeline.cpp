#include <albedo/backend/render-pipeline.h>

namespace albedo
{

namespace backend
{

RenderPipeline::RenderPipeline() noexcept
{
  m_fragmentStageDescriptor = { .entry_point = "main" };

  m_colorStateDescriptor.format = WGPUTextureFormat_Bgra8Unorm;
  m_colorStateDescriptor.alpha_blend = {
    .src_factor = WGPUBlendFactor_One,
    .dst_factor = WGPUBlendFactor_Zero,
    .operation = WGPUBlendOperation_Add,
  };
  m_colorStateDescriptor.color_blend = {
    .src_factor = WGPUBlendFactor_One,
    .dst_factor = WGPUBlendFactor_Zero,
    .operation = WGPUBlendOperation_Add,
  };
  m_colorStateDescriptor.write_mask = WGPUColorWrite_ALL;

  m_descriptor.layout = 0;
  m_descriptor.vertex_stage = { .entry_point = "main" };
  m_descriptor.fragment_stage = &m_fragmentStageDescriptor;
  m_descriptor.primitive_topology = WGPUPrimitiveTopology_TriangleList;
  m_descriptor.color_states = &m_colorStateDescriptor;
  m_descriptor.color_states_length = 1;
  m_descriptor.depth_stencil_state = NULL;
  m_descriptor.sample_count = 1;
}

RenderPipeline::~RenderPipeline()
{
  // TODO: destroy the object.
  // Needs fix from wgpu-native.
}

void
RenderPipeline::create(WGPUDeviceId deviceId, WGPUPipelineLayoutId pipelineLayoutId)
{
  m_descriptor.layout = pipelineLayoutId;
  // TODO: add check for success.
  m_id = wgpu_device_create_render_pipeline(deviceId, &m_descriptor);
}

void
RenderPipeline::bindVertexShader(
  WGPUShaderModuleId module,
  const char* entry
)
{
  m_descriptor.vertex_stage.module = module;
  m_descriptor.vertex_stage.entry_point = entry;
}

void
RenderPipeline::bindFragmentShader(
  WGPUShaderModuleId module,
  const char* entry
)
{
  m_fragmentStageDescriptor.module = module;
  m_fragmentStageDescriptor.entry_point = entry;
}

} // namespace backend

} // namespace albedo
