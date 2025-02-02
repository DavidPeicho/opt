#include <albedo/backend/pipeline.h>

namespace albedo
{

namespace backend
{

PipelineLayout::~PipelineLayout() noexcept
{
// TODO
}

void
PipelineLayout::create(
  WGPUDeviceId deviceId,
  const std::initializer_list<WGPUBindGroupLayoutId>& layouts
)
{
  auto descriptor = WGPUPipelineLayoutDescriptor {
    .bind_group_layouts = nullptr,
    .bind_group_layouts_length = layouts.size()
  };
  descriptor.bind_group_layouts = layouts.size() > 0 ? layouts.begin() : nullptr;
  m_id = wgpu_device_create_pipeline_layout(deviceId, &descriptor);
}

RenderPipeline::RenderPipeline() noexcept
{
  m_fragmentStageDescriptor = { .entry_point = "main" };

  m_rasterizationStateDescriptor.front_face = WGPUFrontFace_Ccw;
  m_rasterizationStateDescriptor.cull_mode = WGPUCullMode_None;
  m_rasterizationStateDescriptor.depth_bias = 0;
  m_rasterizationStateDescriptor.depth_bias_slope_scale = 0.0;
  m_rasterizationStateDescriptor.depth_bias_clamp = 0.0;

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

  m_descriptor.rasterization_state = &m_rasterizationStateDescriptor;

  m_descriptor.primitive_topology = WGPUPrimitiveTopology_TriangleList;

  m_descriptor.color_states = &m_colorStateDescriptor;
  m_descriptor.color_states_length = 1;
  m_descriptor.depth_stencil_state = NULL;

  m_descriptor.vertex_state.index_format = WGPUIndexFormat_Uint16;
  m_descriptor.vertex_state.vertex_buffers = NULL;
  m_descriptor.vertex_state.vertex_buffers_length = 0;

  m_descriptor.sample_count = 1;
}

RenderPipeline::~RenderPipeline() noexcept
{
  // TODO: destroy the object.
  // Needs fix from wgpu-native.
}

void
RenderPipeline::create(WGPUDeviceId deviceId)
{
  m_descriptor.layout = m_layout != nullptr ? m_layout->id() : 0;
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

ComputePipeline::~ComputePipeline() noexcept
{
  // TODO: destroy the object.
}

void
ComputePipeline::create(WGPUDeviceId deviceId)
{
  m_descriptor.layout = m_layout != nullptr ? m_layout->id() : 0;
  // TODO: add check for success.
  m_id = wgpu_device_create_compute_pipeline(deviceId, &m_descriptor);
}

void
ComputePipeline::bindShader(WGPUShaderModuleId module, const char* entry)
{
  m_descriptor.compute_stage.module = module;
  m_descriptor.compute_stage.entry_point = entry;
}

} // namespace backend

} // namespace albedo
