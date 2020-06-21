#include <iostream>
#include <albedo/renderer.h>

namespace albedo
{

namespace
{
  WGPUU32Array readFile(const char *name)
  {
    FILE *file = fopen(name, "rb");
    if (!file) {
        printf("Unable to open %s\n", name);
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    unsigned char *bytes = new unsigned char[length];
    fseek(file, 0, SEEK_SET);
    fread(bytes, 1, length, file);
    fclose(file);
    return (WGPUU32Array){
        .bytes = (uint32_t*) bytes,
        .length = static_cast<uintptr_t>(length / 4),
    };
  }
}

Renderer::Renderer(
  WGPUDeviceId deviceId,
  WGPUSurfaceId surfaceId,
  uint32_t width,
  uint32_t height
)
  : m_deviceId{deviceId}
  , m_surfaceId{surfaceId}
{
  m_info.width = width;
  m_info.height = height;
  m_uniforms.time = 0.0;
  m_swapChainDescriptor.usage = WGPUTextureUsage_OUTPUT_ATTACHMENT;
  m_swapChainDescriptor.format = WGPUTextureFormat_Bgra8Unorm;
  m_swapChainDescriptor.present_mode = WGPUPresentMode_Fifo;
  m_swapChainDescriptor.width = width;
  m_swapChainDescriptor.height = height;
}

Renderer::~Renderer()
{
  // TODO: free resources
}

Renderer&
Renderer::init(const Scene& scene)
{
  WGPUShaderModuleDescriptor vertexModuleDescriptor{
      .code = readFile("../src/shaders/blitting.vert.spv"),
  };
  WGPUShaderModuleDescriptor fragmentModuleDescriptor{
      .code = readFile("../src/shaders/debug.frag.spv"),
  };
  WGPUShaderModuleId vertexShader = wgpu_device_create_shader_module(m_deviceId, &vertexModuleDescriptor);
  WGPUShaderModuleId fragmentShader = wgpu_device_create_shader_module(m_deviceId, &fragmentModuleDescriptor);

  m_bindGroupLayout.setEntries({
    {
        .binding = 0,
        .visibility = WGPUShaderStage_FRAGMENT,
        .ty = WGPUBindingType_UniformBuffer
    },
    {
        .binding = 1,
        .visibility = WGPUShaderStage_FRAGMENT,
        .ty = WGPUBindingType_ReadonlyStorageBuffer
    },
    {
        .binding = 2,
        .visibility = WGPUShaderStage_FRAGMENT,
        .ty = WGPUBindingType_ReadonlyStorageBuffer
    },
    {
        .binding = 3,
        .visibility = WGPUShaderStage_FRAGMENT,
        .ty = WGPUBindingType_ReadonlyStorageBuffer
    },
    {
        .binding = 4,
        .visibility = WGPUShaderStage_FRAGMENT,
        .ty = WGPUBindingType_ReadonlyStorageBuffer
    },
    {
        .binding = 5,
        .visibility = WGPUShaderStage_FRAGMENT,
        .ty = WGPUBindingType_ReadonlyStorageBuffer
    },
    {
        .binding = 6,
        .visibility = WGPUShaderStage_FRAGMENT,
        .ty = WGPUBindingType_UniformBuffer
    }
  });
  m_bindGroupLayout.create(m_deviceId);

  WGPUBindGroupLayoutId bind_group_layouts[1] = { m_bindGroupLayout.id() };

  WGPUPipelineLayoutDescriptor pipelineLayoutDesc{
    .bind_group_layouts = bind_group_layouts,
    .bind_group_layouts_length = 1,
  };

  // TODO: make that part of render pipeline?
  WGPUPipelineLayoutId pipelineLayoutId = wgpu_device_create_pipeline_layout(m_deviceId, &pipelineLayoutDesc);

  m_renderPipeline.bindVertexShader(vertexShader, "main");
  m_renderPipeline.bindFragmentShader(fragmentShader, "main");
  m_renderPipeline.create(m_deviceId, pipelineLayoutId);

  m_info.instanceCount = scene.m_instances.size();
  m_renderInfoBuffer.setUsage(WGPUBufferUsage_COPY_DST | WGPUBufferUsage_UNIFORM);
  m_renderInfoBuffer.setSize(1);
  m_renderInfoBuffer.create(m_deviceId);

  m_instanceBuffer.create(m_deviceId, scene.m_instances);
  m_nodesBuffer.create(m_deviceId, scene.m_nodes);
  m_vertexBuffer.create(m_deviceId, scene.m_vertices);
  m_indicesBuffer.create(m_deviceId, scene.m_indices);
  m_materialBuffer.create(m_deviceId, scene.m_materials);

  m_uniformsBuffer.setUsage(WGPUBufferUsage_COPY_DST | WGPUBufferUsage_UNIFORM);
  m_uniformsBuffer.setSize(1);
  m_uniformsBuffer.create(m_deviceId);

  m_renderTarget.setDescriptor(WGPUTextureDescriptor {
    .label = "render targert",
    .size = WGPUExtent3d { .width = 640, .height = 480, .depth = 1 },
    .mip_level_count = 1,
    .sample_count = 1,
    .dimension = WGPUTextureDimension_D2,
    .format = WGPUTextureFormat_Rgba16Float,
    .usage = WGPUTextureUsage_SAMPLED | WGPUTextureUsage_STORAGE
  });
  m_renderTarget.create(m_deviceId);

  m_bindGroup.setEntries({
    { .binding = 0, .resource = m_renderInfoBuffer.getBindingResource() },
    { .binding = 1, .resource = m_instanceBuffer.getBindingResource() },
    { .binding = 2, .resource = m_nodesBuffer.getBindingResource() },
    { .binding = 3, .resource = m_indicesBuffer.getBindingResource() },
    { .binding = 4, .resource = m_vertexBuffer.getBindingResource() },
    { .binding = 5, .resource = m_materialBuffer.getBindingResource() },
    { .binding = 6, .resource = m_uniformsBuffer.getBindingResource() }
  });

  m_bindGroup.create(m_deviceId, m_bindGroupLayout.id());

  // TODO: check for errors?
  m_swapChainId = wgpu_device_create_swap_chain(
    m_deviceId, m_surfaceId, &m_swapChainDescriptor
  );

  return *this;
}

Renderer&
Renderer::resize(uint32_t width, uint32_t height)
{
  const auto previousWidth = m_info.width;
  const auto previousHeight = m_info.height;
  if (width != previousWidth || height != previousHeight)
  {
    m_info.width = width;
    m_info.height = height;
    m_swapChainDescriptor.width = width;
    m_swapChainDescriptor.height = height;
    m_swapChainId = wgpu_device_create_swap_chain(
      m_deviceId, m_surfaceId, &m_swapChainDescriptor
    );

    WGPUQueueId queue = wgpu_device_get_default_queue(m_deviceId);
    m_renderInfoBuffer.flush(queue, &m_info, 1);
  }
  return *this;
}

Renderer&
Renderer::startFrame(float deltaTime)
{
  WGPUSwapChainOutput nextSwapTexture = wgpu_swap_chain_get_next_texture(m_swapChainId);
  // TODO: add error handling
  if (!nextSwapTexture.view_id)
  {
    std::cout << "Cant acquire swap chaint texture" << std::endl;
    return *this;
  }

  WGPUQueueId queue = wgpu_device_get_default_queue(m_deviceId);

  m_uniforms.time += 1.0 + deltaTime;
  m_uniformsBuffer.flush(queue, &m_uniforms, 1);

  WGPUCommandEncoderDescriptor computeCommandEncoderDescription {
    .label = "command encoder"
  };
  m_commandEncoder = wgpu_device_create_command_encoder(
    m_deviceId, &computeCommandEncoderDescription
  );

  WGPURenderPassColorAttachmentDescriptor
    colorAttachments[1] = {
      {
        .attachment = nextSwapTexture.view_id,
        .load_op = WGPULoadOp_Clear,
        .store_op = WGPUStoreOp_Store,
        .clear_color = WGPUColor_GREEN,
      },
  };

  WGPURenderPassDescriptor renderPassDesc {
    .color_attachments = colorAttachments,
    .color_attachments_length = 1,
    .depth_stencil_attachment = NULL,
  };

  WGPURenderPassId rpass = wgpu_command_encoder_begin_render_pass(m_commandEncoder, &renderPassDesc);
  wgpu_render_pass_set_pipeline(rpass, m_renderPipeline.id());
  wgpu_render_pass_set_bind_group(rpass, 0, m_bindGroup.id(), NULL, 0);
  wgpu_render_pass_draw(rpass, 3, 1, 0, 0);
  wgpu_render_pass_end_pass(rpass);

  return *this;
}

Renderer&
Renderer::endFrame()
{
  WGPUQueueId queue = wgpu_device_get_default_queue(m_deviceId);
  auto cmdBuffer = wgpu_command_encoder_finish(m_commandEncoder, NULL);
  wgpu_queue_submit(queue, const_cast<const WGPUCommandBufferId*>(&cmdBuffer), 1);
  wgpu_swap_chain_present(m_swapChainId);
  return *this;
}


} // namespace albedo
