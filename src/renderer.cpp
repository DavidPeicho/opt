#include <memory>
#include <iostream>

#include <albedo/renderer.h>

#define ACCUMULATE

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
    const backend::BindGroupLayout::Ptr& uniformsBindGroupLayout,
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
          .ty = WGPUBindingType_ReadonlyStorageBuffer
      },
      {
          .binding = 7,
          .visibility = WGPUShaderStage_COMPUTE,
          .ty = WGPUBindingType_UniformBuffer
      },
      {
        .binding = 8,
        .visibility = WGPUShaderStage_COMPUTE,
        .ty = WGPUBindingType_ReadonlyStorageTexture
      },
      {
        .binding = 9,
        .visibility = WGPUShaderStage_COMPUTE,
        .ty = WGPUBindingType_WriteonlyStorageTexture
      }
    });

    bindGroup.setLayout(bindGroupLayout);

    auto pipelineLayout = std::make_shared<backend::PipelineLayout>();
    pipelineLayout->create(deviceId, {
      bindGroupLayout->id(),
      uniformsBindGroupLayout->id()
    });

    WGPUShaderModuleDescriptor moduleDescriptor {
      .code = readFile("./src/shaders/debug.comp.spv"),
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
      .code = readFile("./src/shaders/blitting.vert.spv"),
    };
    WGPUShaderModuleDescriptor fragModuleDescriptor{
      .code = readFile("./src/shaders/blitting.frag.spv"),
    };
    WGPUShaderModuleId vertexShader = wgpu_device_create_shader_module(deviceId, &vertexModuleDescriptor);
    WGPUShaderModuleId fragmentShader = wgpu_device_create_shader_module(deviceId, &fragModuleDescriptor);

    pipeline.bindVertexShader(vertexShader, "main");
    pipeline.bindFragmentShader(fragmentShader, "main");
    pipeline.setPipelineLayout(pipelineLayout);
    pipeline.create(deviceId);
  }
}

Renderer::Renderer() noexcept
{
  m_info.width = 0;
  m_info.height = 0;
  m_info.frameCount = 1;
  m_uniforms.time = 0.0;
  m_swapChainDescriptor.usage = WGPUTextureUsage_OUTPUT_ATTACHMENT;
  m_swapChainDescriptor.format = WGPUTextureFormat_Bgra8Unorm;
  m_swapChainDescriptor.present_mode = WGPUPresentMode_Fifo;
  m_swapChainDescriptor.width = 0;
  m_swapChainDescriptor.height = 0;
}

Renderer::~Renderer() noexcept
{
  // TODO: free resources
}

Renderer&
Renderer::init(WGPUDeviceId deviceId, WGPUSurfaceId surfaceId)
{
  // TODO: delete data if deviceId changes.
  // TODO: delete data if surfaceId changes.
  m_deviceId = deviceId;
  m_surfaceId = surfaceId;

  // TODO: check for errors?
  m_swapChainId = wgpu_device_create_swap_chain(
    m_deviceId, m_surfaceId, &m_swapChainDescriptor
  );

  return *this;
}

Renderer&
Renderer::buildTLAS(const Scene& scene)
{
  m_info.instanceCount = scene.m_instances.size();
  m_info.lightCount = scene.m_lights.size();

  m_renderInfoBuffer.setUsage(WGPUBufferUsage_COPY_DST | WGPUBufferUsage_UNIFORM);
  m_renderInfoBuffer.setSize(1);
  m_renderInfoBuffer.create(m_deviceId);

  m_instanceBuffer.create(m_deviceId, scene.m_instances);
  m_nodesBuffer.create(m_deviceId, scene.m_nodes);
  m_vertexBuffer.create(m_deviceId, scene.m_vertices);
  m_indicesBuffer.create(m_deviceId, scene.m_indices);
  m_materialBuffer.create(m_deviceId, scene.m_materials);
  m_lightsBuffer.create(m_deviceId, scene.m_lights);

  m_uniformsBuffer.setUsage(WGPUBufferUsage_COPY_DST | WGPUBufferUsage_UNIFORM);
  m_uniformsBuffer.setSize(1);
  m_uniformsBuffer.create(m_deviceId);

  m_cameraUniformsBuffer.setUsage(WGPUBufferUsage_COPY_DST | WGPUBufferUsage_UNIFORM);
  m_cameraUniformsBuffer.setSize(1);
  m_cameraUniformsBuffer.create(m_deviceId);

  auto uniformsBindGroupLayout = std::make_shared<backend::BindGroupLayout>();
  uniformsBindGroupLayout->create(m_deviceId, {
    {
      .binding = 0,
      .visibility = WGPUShaderStage_COMPUTE,
      .ty = WGPUBindingType_UniformBuffer
    }
  });
  m_cameraBindGroup.setLayout(uniformsBindGroupLayout);
  m_cameraBindGroup.create(m_deviceId, {
    { .binding = 0, .resource = m_cameraUniformsBuffer.getBindingResource() },
  });

  initPathtracingPipeline(
    m_pathtracingPipeline, m_pathtracingBindGroup, uniformsBindGroupLayout,
    m_deviceId
  );
  initBlittingPipeline(m_renderPipeline, m_blittingBindGroup, m_deviceId);

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

    auto rtDescriptor = WGPUTextureDescriptor {
      .label = "render targert",
      .size = WGPUExtent3d { .width = width, .height = height, .depth = 1 },
      .mip_level_count = 1,
      .sample_count = 1,
      .dimension = WGPUTextureDimension_D2,
      .format = WGPUTextureFormat_Rgba32Float,
      .usage = WGPUTextureUsage_SAMPLED | WGPUTextureUsage_STORAGE
    };
    m_renderTarget.setDescriptor(rtDescriptor);
    m_renderTarget.create(m_deviceId);
    m_renderTarget2.setDescriptor(rtDescriptor);
    m_renderTarget2.create(m_deviceId);

    auto view = m_renderTarget.createDefaultView();
    view.create();

    // TODO: remove when light sampling is added to Ray Struct
    auto view2 = m_renderTarget2.createDefaultView();
    view2.create();

    // Needed because the texture changes.
    // TODO: create a separate bindgroup for the texture?
    m_pathtracingBindGroup.create(m_deviceId, {
      { .binding = 0, .resource = m_renderInfoBuffer.getBindingResource() },
      { .binding = 1, .resource = m_instanceBuffer.getBindingResource() },
      { .binding = 2, .resource = m_nodesBuffer.getBindingResource() },
      { .binding = 3, .resource = m_indicesBuffer.getBindingResource() },
      { .binding = 4, .resource = m_vertexBuffer.getBindingResource() },
      { .binding = 5, .resource = m_materialBuffer.getBindingResource() },
      { .binding = 6, .resource = m_lightsBuffer.getBindingResource() },
      { .binding = 7, .resource = m_uniformsBuffer.getBindingResource() },
      { .binding = 8, .resource = view.getBindingResource() },
      { .binding = 9, .resource = view2.getBindingResource() }
  });

    m_pathtracingBindGroup2.setLayout(m_pathtracingBindGroup.getLayout());
    m_pathtracingBindGroup2.create(m_deviceId, {
      { .binding = 0, .resource = m_renderInfoBuffer.getBindingResource() },
      { .binding = 1, .resource = m_instanceBuffer.getBindingResource() },
      { .binding = 2, .resource = m_nodesBuffer.getBindingResource() },
      { .binding = 3, .resource = m_indicesBuffer.getBindingResource() },
      { .binding = 4, .resource = m_vertexBuffer.getBindingResource() },
      { .binding = 5, .resource = m_materialBuffer.getBindingResource() },
      { .binding = 6, .resource = m_lightsBuffer.getBindingResource() },
      { .binding = 7, .resource = m_uniformsBuffer.getBindingResource() },
      { .binding = 8, .resource = view2.getBindingResource() },
      { .binding = 9, .resource = view.getBindingResource() }
    });

    m_blittingBindGroup.create(m_deviceId, {
      { .binding = 0, .resource = m_renderInfoBuffer.getBindingResource() },
      { .binding = 1, .resource = m_rtSampler.getBindingResource() },
      { .binding = 2, .resource = view.getBindingResource() }
    });

    m_info.frameCount = 1;
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

  #ifdef ACCUMULATE
  m_uniforms.time += 1.0 + deltaTime;
  #endif

  m_uniformsBuffer.flush(queue, &m_uniforms, 1);
  // TODO: flush only if camera info changes
  m_cameraUniformsBuffer.flush(queue, &m_cameraUniforms, 1);
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
  #ifdef ACCUMULATE
  const auto& binding = m_info.frameCount % 2 == 0 ?
    m_pathtracingBindGroup : m_pathtracingBindGroup2;
  wgpu_compute_pass_set_bind_group(computePassId, 0, binding.id(), NULL, 0);
  wgpu_compute_pass_set_bind_group(computePassId, 1, m_cameraBindGroup.id(), NULL, 0);
  #else
  wgpu_compute_pass_set_bind_group(computePassId, 0, m_pathtracingBindGroup2.id(), NULL, 0);
  wgpu_compute_pass_set_bind_group(computePassId, 1, m_cameraBindGroup.id(), NULL, 0);
  #endif
  wgpu_compute_pass_dispatch(computePassId, m_info.width / 8, m_info.height / 8, 1);
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

  #ifdef ACCUMULATE
  m_info.frameCount += 1;
  #endif

  return *this;
}

Renderer&
Renderer::setCameraInfo(
  const components::PerspectiveCamera& camera,
  const glm::vec3& origin,
  const glm::vec3& up,
  const glm::vec3& right
)
{
  m_cameraUniforms.origin = origin;
  m_cameraUniforms.up = up;
  m_cameraUniforms.right = right;
  m_cameraUniforms.vFOV = camera.vFOV;
  m_info.frameCount = 1;
  return *this;
}

} // namespace albedo
