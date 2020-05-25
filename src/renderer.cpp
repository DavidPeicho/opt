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
Renderer::init()
{
  WGPUShaderModuleDescriptor vertexModuleDescriptor{
      .code = readFile("../src/shaders/blitting.vert.spv"),
  };
  WGPUShaderModuleDescriptor fragmentModuleDescriptor{
      .code = readFile("../src/shaders/blitting.frag.spv"),
  };
  WGPUShaderModuleId vertexShader = wgpu_device_create_shader_module(m_deviceId, &vertexModuleDescriptor);
  WGPUShaderModuleId fragmentShader = wgpu_device_create_shader_module(m_deviceId, &fragmentModuleDescriptor);

  WGPUBindGroupLayoutDescriptor bindLayoutGroupDesriptor {
      .label = "bind group layout",
      .entries = NULL,
      .entries_length = 0,
  };
  WGPUBindGroupLayoutId bindLayoutGroupId = wgpu_device_create_bind_group_layout(m_deviceId, &bindLayoutGroupDesriptor);

  WGPUBindGroupLayoutId bind_group_layouts[1] = { bindLayoutGroupId };

  WGPUPipelineLayoutDescriptor pipelineLayoutDesc{
    .bind_group_layouts = bind_group_layouts,
    .bind_group_layouts_length = 1,
  };

  // TODO: make that part of render pipeline?
  WGPUPipelineLayoutId pipelineLayoutId = wgpu_device_create_pipeline_layout(m_deviceId, &pipelineLayoutDesc);

  m_renderPipeline.bindVertexShader(vertexShader, "main");
  m_renderPipeline.bindFragmentShader(fragmentShader, "main");
  m_renderPipeline.create(m_deviceId, pipelineLayoutId);

  m_bindGroup.create(m_deviceId, bindLayoutGroupId);

  // TODO: check for errors?
  m_swapChainId = wgpu_device_create_swap_chain(
    m_deviceId, m_surfaceId, &m_swapChainDescriptor
  );

  return *this;
}

Renderer&
Renderer::resize(uint32_t width, uint32_t height)
{
  const auto previousWidth = m_swapChainDescriptor.width;
  const auto previousHeight = m_swapChainDescriptor.height;
  if (width != previousWidth || height != previousHeight)
  {
    m_swapChainDescriptor.width = width;
    m_swapChainDescriptor.height = height;
    m_swapChainId = wgpu_device_create_swap_chain(
      m_deviceId, m_surfaceId, &m_swapChainDescriptor
    );
  }
  return *this;
}

Renderer&
Renderer::startFrame()
{
  WGPUSwapChainOutput nextSwapTexture = wgpu_swap_chain_get_next_texture(m_swapChainId);
  // TODO: add error handling
  if (!nextSwapTexture.view_id)
  {
    std::cout << "Cant acquire swap chaint texture" << std::endl;
    return *this;
  }

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
