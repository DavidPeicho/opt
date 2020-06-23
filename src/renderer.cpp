#include <memory>
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

  void
  initPathtracingPipeline(
    backend::ComputePipeline& pipeline,
    backend::BindGroup& bindGroup,
    WGPUDeviceId deviceId
  )
  {
    auto bindGroupLayout = std::make_shared<backend::BindGroupLayout>();
    bindGroupLayout->create(deviceId, {
      {
          .binding = 0,
          .visibility = WGPUShaderStage_COMPUTE,
          .ty = WGPUBindingType_UniformBuffer
      },
      {
          .binding = 1,
          .visibility = WGPUShaderStage_COMPUTE,
          .ty = WGPUBindingType_ReadonlyStorageBuffer
      },
      {
          .binding = 2,
          .visibility = WGPUShaderStage_COMPUTE,
          .ty = WGPUBindingType_ReadonlyStorageBuffer
      },
      {
          .binding = 3,
          .visibility = WGPUShaderStage_COMPUTE,
          .ty = WGPUBindingType_ReadonlyStorageBuffer
      },
      {
          .binding = 4,
          .visibility = WGPUShaderStage_COMPUTE,
          .ty = WGPUBindingType_ReadonlyStorageBuffer
      },
      {
          .binding = 5,
          .visibility = WGPUShaderStage_COMPUTE,
          .ty = WGPUBindingType_ReadonlyStorageBuffer
      },
      {
          .binding = 6,
          .visibility = WGPUShaderStage_COMPUTE,
          .ty = WGPUBindingType_UniformBuffer
      },
      {
        .binding = 7,
        .visibility = WGPUShaderStage_COMPUTE,
        .ty = WGPUBindingType_WriteonlyStorageTexture
      }
    });

    bindGroup.setLayout(bindGroupLayout);

    auto pipelineLayout = std::make_shared<backend::PipelineLayout>();
    pipelineLayout->create(deviceId, { bindGroupLayout->id() });

    WGPUShaderModuleDescriptor moduleDescriptor {
      .code = readFile("../src/shaders/debug.comp.spv"),
    };
    WGPUShaderModuleId module = wgpu_device_create_shader_module(
      deviceId, &moduleDescriptor
    );

    pipeline.bindShader(module, "main");
    pipeline.setPipelineLayout(pipelineLayout);
    pipeline.create(deviceId);
  }

  void
  initBlittingPipeline(
    backend::RenderPipeline& pipeline,
    backend::BindGroup& bindGroup,
    WGPUDeviceId deviceId
  )
  {
    auto bindGroupLayout = std::make_shared<backend::BindGroupLayout>();
    bindGroupLayout->create(deviceId, {
      {
        .binding = 0,
        .visibility = WGPUShaderStage_FRAGMENT,
        .ty = WGPUBindingType_UniformBuffer
      },
      {
        .binding = 1,
        .visibility = WGPUShaderStage_FRAGMENT,
        .ty = WGPUBindingType_Sampler
      },
      {
        .binding = 2,
        .visibility = WGPUShaderStage_FRAGMENT,
        .ty = WGPUBindingType_SampledTexture
      }
    });

    bindGroup.setLayout(bindGroupLayout);

    auto pipelineLayout = std::make_shared<backend::PipelineLayout>();
    pipelineLayout->create(deviceId, { bindGroupLayout->id() });

    WGPUShaderModuleDescriptor vertexModuleDescriptor{
      .code = readFile("../src/shaders/blitting.vert.spv"),
    };
    WGPUShaderModuleDescriptor fragModuleDescriptor{
      .code = readFile("../src/shaders/blitting.frag.spv"),
    };
    WGPUShaderModuleId vertexShader = wgpu_device_create_shader_module(deviceId, &vertexModuleDescriptor);
    WGPUShaderModuleId fragmentShader = wgpu_device_create_shader_module(deviceId, &fragModuleDescriptor);

    pipeline.bindVertexShader(vertexShader, "main");
    pipeline.bindFragmentShader(fragmentShader, "main");
    pipeline.setPipelineLayout(pipelineLayout);
    pipeline.create(deviceId);
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
  m_info.frameCount = 1;
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
  initPathtracingPipeline(m_pathtracingPipeline, m_pathtracingBindGroup, m_deviceId);
  initBlittingPipeline(m_renderPipeline, m_blittingBindGroup, m_deviceId);

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

  // TODO: check for errors?
  m_swapChainId = wgpu_device_create_swap_chain(
    m_deviceId, m_surfaceId, &m_swapChainDescriptor
  );

  m_rtSampler.setDescriptor(WGPUSamplerDescriptor {
    .address_mode_u = WGPUAddressMode_ClampToEdge,
    .address_mode_v = WGPUAddressMode_ClampToEdge,
    .address_mode_w = WGPUAddressMode_ClampToEdge,
    .mag_filter = WGPUFilterMode_Nearest,
    .min_filter = WGPUFilterMode_Nearest,
    .mipmap_filter = WGPUFilterMode_Nearest,
    .lod_min_clamp = 0,
    .lod_max_clamp = 0xffffffff,
    .compare = WGPUCompareFunction_Equal
  });
  m_rtSampler.create(m_deviceId);

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

    auto view = m_renderTarget.createView();
    view.createAsDefault();

    // Needed because the texture changes.
    // TODO: create a separate bindgroup for the texture?
    m_pathtracingBindGroup.create(m_deviceId, {
      { .binding = 0, .resource = m_renderInfoBuffer.getBindingResource() },
      { .binding = 1, .resource = m_instanceBuffer.getBindingResource() },
      { .binding = 2, .resource = m_nodesBuffer.getBindingResource() },
      { .binding = 3, .resource = m_indicesBuffer.getBindingResource() },
      { .binding = 4, .resource = m_vertexBuffer.getBindingResource() },
      { .binding = 5, .resource = m_materialBuffer.getBindingResource() },
      { .binding = 6, .resource = m_uniformsBuffer.getBindingResource() },
      { .binding = 7, .resource = view.getBindingResource() }
    });
    m_blittingBindGroup.create(m_deviceId, {
      { .binding = 0, .resource = m_renderInfoBuffer.getBindingResource() },
      { .binding = 1, .resource = m_rtSampler.getBindingResource() },
      { .binding = 2, .resource = view.getBindingResource() }
    });
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

  // TODO: separate Frame Count from dimensions, and only update the uniform
  // buffer of the framecount. Maybe merge framecount and delta?
  m_renderInfoBuffer.flush(queue, &m_info, 1);

  WGPUCommandEncoderDescriptor computeCommandEncoderDescription {
    .label = "command encoder"
  };

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

  m_commandEncoder = wgpu_device_create_command_encoder(
    m_deviceId, &computeCommandEncoderDescription
  );

  // 1. Run pathtracing

  WGPUComputePassId computePassId = wgpu_command_encoder_begin_compute_pass(
    m_commandEncoder, NULL
  );
  wgpu_compute_pass_set_pipeline(computePassId, m_pathtracingPipeline.id());
  wgpu_compute_pass_set_bind_group(computePassId, 0, m_pathtracingBindGroup.id(), NULL, 0);
  wgpu_compute_pass_dispatch(computePassId, m_info.width, m_info.height, 1);
  wgpu_compute_pass_end_pass(computePassId);

  // 2. Blit to screen

  WGPURenderPassId rpass = wgpu_command_encoder_begin_render_pass(m_commandEncoder, &renderPassDesc);
  wgpu_render_pass_set_pipeline(rpass, m_renderPipeline.id());
  wgpu_render_pass_set_bind_group(rpass, 0, m_blittingBindGroup.id(), NULL, 0);
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

  m_info.frameCount += 1;

  return *this;
}


} // namespace albedo
