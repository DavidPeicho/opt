#include <memory>
#include <iostream>

#include <albedo/renderer.h>

#define ACCUMULATE

namespace albedo
{

namespace
{
  WGPUShaderSource
  readFile(const char *name)
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
    return (WGPUShaderSource){
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
        .ty = WGPUBindingType_ReadonlyStorageTexture,
        .has_dynamic_offset = false,
        .multisampled = false,
        .view_dimension = WGPUTextureViewDimension_D2,
        .texture_component_type = WGPUTextureComponentType_Float
      },
      {
        .binding = 9,
        .visibility = WGPUShaderStage_COMPUTE,
        .ty = WGPUBindingType_WriteonlyStorageTexture,
        .has_dynamic_offset = false,
        .multisampled = false,
        .view_dimension = WGPUTextureViewDimension_D2,
        .texture_component_type = WGPUTextureComponentType_Float
      },
      {
        .binding = 10,
        .visibility = WGPUShaderStage_COMPUTE,
        .ty = WGPUBindingType_SampledTexture,
        .has_dynamic_offset = false,
        .multisampled = false,
        .view_dimension = WGPUTextureViewDimension_D2,
        .texture_component_type = WGPUTextureComponentType_Float
      },
      {
        .binding = 11,
        .visibility = WGPUShaderStage_COMPUTE,
        .ty = WGPUBindingType_Sampler
      }
    });

    bindGroup.setLayout(bindGroupLayout);

    auto pipelineLayout = std::make_shared<backend::PipelineLayout>();
    pipelineLayout->create(deviceId, {
      bindGroupLayout->id(),
      uniformsBindGroupLayout->id()
    });

    auto comp = readFile("./src/shaders/pathtrace.comp.spv");
    WGPUShaderModuleId module = wgpu_device_create_shader_module(
      deviceId, &comp
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

    const auto vs = readFile("./src/shaders/blitting.vert.spv");
    WGPUShaderModuleId vertexShader = wgpu_device_create_shader_module(
      deviceId, &vs
    );
    const auto fs = readFile("./src/shaders/blitting.frag.spv");
    WGPUShaderModuleId fragmentShader = wgpu_device_create_shader_module(
      deviceId, &fs
    );

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
  m_swapChainDescriptor.usage = WGPUTextureUsage_RENDER_ATTACHMENT;
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

  // The code to upload texture was either wrong on my side, or wrong in the
  // wgpu version I had. I didn't have time to investigate that further.

  // if (scene.m_texturesInfo.textures.size() > 0)
  // {
  //   const uint32_t width = scene.m_texturesInfo.width;
  //   const uint32_t height = scene.m_texturesInfo.height;
  //   const uint32_t depth = scene.m_texturesInfo.textures.size();

  //   std::cout << "NB textures = " << depth << std::endl;
  //   std::cout << "Texture size = (" << width << ", " << height << ")" << std::endl;

  //   // TODO: dispose textures when uploaded?
  //   m_textures.setDescriptor(WGPUTextureDescriptor {
  //     .label = "textures",
  //     .size = WGPUExtent3d { .width = width, .height = height, .depth = depth },
  //     .mip_level_count = 1,
  //     .sample_count = 1,
  //     .dimension = WGPUTextureDimension_D2,
  //     .format = WGPUTextureFormat_Rgba8Uint,
  //     .usage = WGPUTextureUsage_SAMPLED | WGPUTextureUsage_COPY_DST
  //   });

  //   m_textures.create(m_deviceId);

  //   auto queue = wgpu_device_get_default_queue(m_deviceId);

  //   const auto extent = WGPUExtent3d { .width = width, .height = height, .depth = 1 };

  //   for (uint32_t i = 0; i < depth; ++i) {

  //     auto view = WGPUTextureCopyView {
  //       .texture = m_probe.id(),
  //       .mip_level = 0,
  //       .origin = { .x = 0, .y = 0, .z = i },
  //     };
  //     const auto* data = &scene.m_texturesInfo.textures[i][0];
  //     // TODO: this assumes the images are RGBA...
  //     // TODO: this assumes the images are unsigned char...
  //     uint32_t bytes_per_row = 4 * sizeof (unsigned char) * width;
  //     uint32_t bytes_total = bytes_per_row * height;

  //     std::cout << bytes_total << std::endl;

  //     auto layout = WGPUTextureDataLayout {
  //       .offset = 0,
  //       .bytes_per_row = bytes_per_row,
  //       .rows_per_image = height
  //     };
  //     wgpu_queue_write_texture(
  //       queue,
  //       &view,
  //       reinterpret_cast<const uint8_t*>(data),
  //       bytes_total,
  //       &layout,
  //       &extent
  //     );
  //   }
  // }

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
    m_cameraUniformsBuffer.createBindGroupEntry(0)
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
    .compare = WGPUCompareFunction_Undefined
  });
  m_rtSampler.create(m_deviceId);

  m_probeSampler.setDescriptor(WGPUSamplerDescriptor {
    .address_mode_u = WGPUAddressMode_ClampToEdge,
    .address_mode_v = WGPUAddressMode_ClampToEdge,
    .address_mode_w = WGPUAddressMode_ClampToEdge,
    .mag_filter = WGPUFilterMode_Linear,
    .min_filter = WGPUFilterMode_Linear,
    .mipmap_filter = WGPUFilterMode_Nearest,
    .lod_min_clamp = 0,
    .lod_max_clamp = 0xffffffff,
    .compare = WGPUCompareFunction_Undefined
  });
  m_probeSampler.create(m_deviceId);

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

    auto probeView = m_probe.createDefaultView();
    probeView.create();

    // Needed because the texture changes.
    // TODO: create a separate bindgroup for the texture?
    m_pathtracingBindGroup.create(m_deviceId, {
      m_renderInfoBuffer.createBindGroupEntry(0),
      m_instanceBuffer.createBindGroupEntry(1),
      m_nodesBuffer.createBindGroupEntry(2),
      m_indicesBuffer.createBindGroupEntry(3),
      m_vertexBuffer.createBindGroupEntry(4),
      m_materialBuffer.createBindGroupEntry(5),
      m_lightsBuffer.createBindGroupEntry(6),
      m_uniformsBuffer.createBindGroupEntry(7),
      view.createBindGroupEntry(8),
      view2.createBindGroupEntry(9),
      probeView.createBindGroupEntry(10),
      m_probeSampler.createBindGroupEntry(11)
    });

    m_pathtracingBindGroup2.setLayout(m_pathtracingBindGroup.getLayout());
    m_pathtracingBindGroup2.create(m_deviceId, {
      m_renderInfoBuffer.createBindGroupEntry(0),
      m_instanceBuffer.createBindGroupEntry(1),
      m_nodesBuffer.createBindGroupEntry(2),
      m_indicesBuffer.createBindGroupEntry(3),
      m_vertexBuffer.createBindGroupEntry(4),
      m_materialBuffer.createBindGroupEntry(5),
      m_lightsBuffer.createBindGroupEntry(6),
      m_uniformsBuffer.createBindGroupEntry(7),
      view2.createBindGroupEntry(8),
      view.createBindGroupEntry(9),
      probeView.createBindGroupEntry(10),
      m_probeSampler.createBindGroupEntry(11)
    });

    m_blittingBindGroup.create(m_deviceId, {
      m_renderInfoBuffer.createBindGroupEntry(0),
      m_rtSampler.createBindGroupEntry(1),
      view.createBindGroupEntry(2)
    });

    m_info.frameCount = 1;
  }
  return *this;
}

Renderer&
Renderer::startFrame(float deltaTime)
{
  WGPUOption_TextureViewId nextSwapTexture = wgpu_swap_chain_get_current_texture_view(m_swapChainId);
  // TODO: add error handling
  if (!nextSwapTexture)
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

  WGPUColorAttachmentDescriptor colorAttachments[1] = {
      {
        .attachment = nextSwapTexture,
        .resolve_target = 0,
        .channel = {
            .load_op = WGPULoadOp_Clear,
            .store_op = WGPUStoreOp_Store,
            .clear_value = WGPUColor_GREEN,
            .read_only = false,
        }
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

  WGPUComputePass *computePassId = wgpu_command_encoder_begin_compute_pass(
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

  WGPURenderPass *rpass = wgpu_command_encoder_begin_render_pass(m_commandEncoder, &renderPassDesc);
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

Renderer&
Renderer::setProbe(float *data, uint width, uint height, uint comp)
{
  auto extent = WGPUExtent3d { .width = width, .height = height, .depth = 1 };
  auto rtDescriptor = WGPUTextureDescriptor {
    .label = "probe",
    .size = extent,
    .mip_level_count = 1,
    .sample_count = 1,
    .dimension = WGPUTextureDimension_D2,
    .format = WGPUTextureFormat_Rgba32Float,
    .usage = WGPUTextureUsage_SAMPLED | WGPUTextureUsage_COPY_DST
  };
  m_probe.setDescriptor(rtDescriptor);
  m_probe.create(m_deviceId);

  auto queue = wgpu_device_get_default_queue(m_deviceId);
  auto view = WGPUTextureCopyView {
    .texture = m_probe.id(),
    .mip_level = 0,
    .origin = WGPUOrigin3d_ZERO,
  };
  uint32_t bytes_per_row = 4 * sizeof (float) * width;
  auto layout = WGPUTextureDataLayout {
    .offset = 0,
    .bytes_per_row = bytes_per_row,
    .rows_per_image = height
  };
  wgpu_queue_write_texture(
    queue,
    &view,
    reinterpret_cast<uint8_t*>(data),
    bytes_per_row * height,
    &layout,
    &extent
  );
  return *this;
}

} // namespace albedo
