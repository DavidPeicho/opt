/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WGPU_H
#define WGPU_H
    #include "wgpu.h"
#endif

#include <framework.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "glm/glm.hpp"

#define WGPU_TARGET_MACOS 1
#define WGPU_TARGET_LINUX_X11 2
#define WGPU_TARGET_WINDOWS 3
#define WGPU_TARGET_LINUX_WAYLAND 4

#if WGPU_TARGET == WGPU_TARGET_MACOS
    #include <Foundation/Foundation.h>
    #include <QuartzCore/CAMetalLayer.h>
#endif

#include <GLFW/glfw3.h>
#if WGPU_TARGET == WGPU_TARGET_MACOS
#define GLFW_EXPOSE_NATIVE_COCOA
#elif WGPU_TARGET == WGPU_TARGET_LINUX_X11
#define GLFW_EXPOSE_NATIVE_X11
#elif WGPU_TARGET == WGPU_TARGET_LINUX_WAYLAND
#define GLFW_EXPOSE_NATIVE_WAYLAND
#elif WGPU_TARGET == WGPU_TARGET_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

#define BLEND_STATES_LENGTH (1)
#define ATTACHMENTS_LENGTH (1)
#define RENDER_PASS_ATTACHMENTS_LENGTH (1)
#define BIND_GROUP_LAYOUTS_LENGTH (1)

void request_adapter_callback(WGPUAdapterId received, void* userdata) {
    *(WGPUAdapterId*)userdata = received;
}

WGPUBufferId
createInputBuffer(WGPUDeviceId deviceId, uint16_t width, uint16_t height)
{
    uint8_t *staging_memory;

    size_t nbElements = width * height;

    WGPUBufferDescriptor descriptor {
        .label = "buffer",
        .size = nbElements * sizeof(glm::vec3),
        .usage = WGPUBufferUsage_STORAGE_READ,
    };

    WGPUBufferId buffer = wgpu_device_create_buffer_mapped(deviceId, &descriptor, &staging_memory);

    glm::vec3* ptr = (glm::vec3*)staging_memory;
    for (size_t i = 0; i < nbElements; ++i) {
        ptr[i] = glm::vec3(1.0, 0.0, 0.0);
    }

	wgpu_buffer_unmap(buffer);

    return buffer;
}

WGPUBufferId
createOutputBuffer(WGPUDeviceId deviceId, uint16_t width, uint16_t height)
{
    uint8_t *staging_memory;

    size_t nbElements = width * height;

    WGPUBufferDescriptor descriptor {
        .label = "buffer",
        .size = nbElements * sizeof(glm::vec3),
        .usage = WGPUBufferUsage_STORAGE,
    };

    WGPUBufferId buffer = wgpu_device_create_buffer(deviceId, &descriptor);
    return buffer;
}

WGPUComputePipelineId createComputePipeline(
    WGPUDeviceId deviceId,
    uint16_t width,
    uint16_t height,
    WGPUBindGroupId* bindGroupId
)
{
    size_t nbElements = width * height;

    // Creates input buffer
    WGPUBindingResource inputResource = {
		.tag = WGPUBindingResource_Buffer,
        .buffer = (WGPUBufferBinding) {
            .buffer = createInputBuffer(deviceId, width, height),
            .size = nbElements * sizeof(glm::vec3),
            .offset = 0
        }
    };

    // Creates output resource
    WGPUBindingResource outputResource = {
		.tag = WGPUBindingResource_Buffer,
        .buffer = (WGPUBufferBinding) {
            .buffer = createOutputBuffer(deviceId, width, height),
            .size = nbElements * sizeof(glm::vec3),
            .offset = 0
        }
    };

    const WGPUBindGroupLayoutEntry layoutEntries[2] = {
        {
            .binding = 0,
            .visibility = WGPUShaderStage_COMPUTE,
            .ty = WGPUBindingType_ReadonlyStorageBuffer
        },
        {
            .binding = 1,
            .visibility = WGPUShaderStage_COMPUTE,
            .ty = WGPUBindingType_StorageBuffer
        },

    };

    const WGPUBindGroupEntry entries[2] = {
        {
            .binding = 0,
			.resource = inputResource
        },
        {
            .binding = 1,
			.resource = outputResource
        },

    };

    WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor {
        .label = "bind group layout",
        .entries = layoutEntries,
        .entries_length = 2
    };

    WGPUBindGroupLayoutId bindGroupLayout = wgpu_device_create_bind_group_layout(deviceId, &bindGroupLayoutDescriptor);
    WGPUBindGroupLayoutId bindGroupLayouts[1] = {
        bindGroupLayout
    };

    WGPUBindGroupDescriptor bindGroupDescriptor {
        .label = "bind group",
        .layout = bindGroupLayout,
        .entries = entries,
        .entries_length = 2
    };

    *bindGroupId = wgpu_device_create_bind_group(deviceId, &bindGroupDescriptor);

    WGPUPipelineLayoutDescriptor pipelineLayoutDescriptor {
        .bind_group_layouts = bindGroupLayouts,
        .bind_group_layouts_length = 1
    };

    WGPUPipelineLayoutId pipeline_layout = wgpu_device_create_pipeline_layout(deviceId, &pipelineLayoutDescriptor);

    WGPUShaderModuleDescriptor shaderModuleDescriptor {
        .code = read_file("../src/shaders/pathtracing.comp.spv")
    };

    WGPUShaderModuleId shader_module = wgpu_device_create_shader_module(deviceId, &shaderModuleDescriptor);

    WGPUComputePipelineDescriptor pipelineDescriptor {
        .layout = pipeline_layout,
        .compute_stage = (WGPUProgrammableStageDescriptor) {
            .module = shader_module,
            .entry_point = "main"
        }
    };

    return wgpu_device_create_compute_pipeline(deviceId, &pipelineDescriptor);
}

int main() {
    if (!glfwInit()) {
        printf("Cannot initialize glfw");
        return 1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(640, 480, "wgpu with glfw", NULL, NULL);

    if (!window) {
        printf("Cannot create window");
        return 1;
    }

    WGPUSurfaceId surface;

#if WGPU_TARGET == WGPU_TARGET_MACOS
    {
        id metal_layer = NULL;
        NSWindow* ns_window = glfwGetCocoaWindow(window);
        [ns_window.contentView setWantsLayer : YES] ;
        metal_layer = [CAMetalLayer layer];
        [ns_window.contentView setLayer : metal_layer] ;
        surface = wgpu_create_surface_from_metal_layer(metal_layer);
    }
#else
  #error "Unsupported WGPU_TARGET"
#endif

    WGPUAdapterId adapter = { 0 };

    WGPURequestAdapterOptions adapterOptions {
      .power_preference = WGPUPowerPreference_LowPower,
      .compatible_surface = surface
    };

    wgpu_request_adapter_async(
        &adapterOptions,
        2 | 4 | 8,
        request_adapter_callback,
        (void*)&adapter
    );

    WGPUDeviceDescriptor deviceDescriptor{
        .extensions =
        {
            .anisotropic_filtering = false,
        },
        .limits =
        {
            .max_bind_groups = 1,
        },
    };

    WGPUDeviceId device = wgpu_adapter_request_device(adapter, &deviceDescriptor);

    WGPUShaderModuleDescriptor vertexModuleDescriptor{
        .code = read_file("../triangle.vert.spv"),
    };
    WGPUShaderModuleDescriptor fragmentModuleDescriptor{
        .code = read_file("../triangle.frag.spv"),
    };

    WGPUShaderModuleId vertex_shader = wgpu_device_create_shader_module(device, &vertexModuleDescriptor);
    WGPUShaderModuleId fragment_shader = wgpu_device_create_shader_module(device, &fragmentModuleDescriptor);

    WGPUBindGroupLayoutDescriptor bindLayoutGroup {
        .label = "bind group layout",
            .entries = NULL,
            .entries_length = 0,
    };

    WGPUBindGroupLayoutId bindLayoutGroupId = wgpu_device_create_bind_group_layout(device, &bindLayoutGroup);

    WGPUBindGroupDescriptor bindGroupDesc{
        .label = "bind group",
        .layout = bindLayoutGroupId,
        .entries = NULL,
        .entries_length = 0,
    };

    WGPUBindGroupId bindGroupId = wgpu_device_create_bind_group(device, &bindGroupDesc);

    WGPUBindGroupLayoutId bind_group_layouts[BIND_GROUP_LAYOUTS_LENGTH] = { bindLayoutGroupId };

    WGPUPipelineLayoutDescriptor pipelineLayoutDesc{
        .bind_group_layouts = bind_group_layouts,
        .bind_group_layouts_length = BIND_GROUP_LAYOUTS_LENGTH,
    };

    WGPUPipelineLayoutId pipeline_layout = wgpu_device_create_pipeline_layout(device, &pipelineLayoutDesc);

    WGPUProgrammableStageDescriptor fragmentStage {
        .module = fragment_shader,
        .entry_point = "main"
    };

    WGPUColorStateDescriptor colorStateDescriptor{
        .format = WGPUTextureFormat_Bgra8Unorm,
        .alpha_blend = {
            .src_factor = WGPUBlendFactor_One,
            .dst_factor = WGPUBlendFactor_Zero,
            .operation = WGPUBlendOperation_Add,
        },
        .color_blend = {
            .src_factor = WGPUBlendFactor_One,
            .dst_factor = WGPUBlendFactor_Zero,
            .operation = WGPUBlendOperation_Add,
        },
        .write_mask = WGPUColorWrite_ALL,
    };

    WGPURasterizationStateDescriptor rasterizationStateDes{
        .front_face = WGPUFrontFace_Ccw,
        .cull_mode = WGPUCullMode_None,
        .depth_bias = 0,
        .depth_bias_slope_scale = 0.0,
        .depth_bias_clamp = 0.0,
    };

    WGPURenderPipelineDescriptor renderPipelineDesc {
        .layout = pipeline_layout,
        .vertex_stage = { .module = vertex_shader, .entry_point = "main" },
        .fragment_stage = &fragmentStage,
        .rasterization_state = &rasterizationStateDes,
        .vertex_state = {
            .index_format = WGPUIndexFormat_Uint16,
            .vertex_buffers = NULL,
            .vertex_buffers_length = 0,
        },
        .primitive_topology = WGPUPrimitiveTopology_TriangleList,
        .color_states = &colorStateDescriptor,
        .color_states_length = 1,
        .depth_stencil_state = NULL,
        .sample_count = 1,
    };

    WGPURenderPipelineId render_pipeline = wgpu_device_create_render_pipeline(device, &renderPipelineDesc);

    WGPUBindGroupId computeBingGroupId;
    auto computePipeline = createComputePipeline(device, 640, 480, &computeBingGroupId);

    int prev_width = 0;
    int prev_height = 0;
    glfwGetWindowSize(window, &prev_width, &prev_height);

    WGPUSwapChainDescriptor swapChainDesc{
        .usage = WGPUTextureUsage_OUTPUT_ATTACHMENT,
        .format = WGPUTextureFormat_Bgra8Unorm,
        .width = static_cast<uint32_t>(prev_width),
        .height = static_cast<uint32_t>(prev_height),
        .present_mode = WGPUPresentMode_Fifo,
    };

    WGPUSwapChainId swap_chain = wgpu_device_create_swap_chain(device, surface, &swapChainDesc);

    while (!glfwWindowShouldClose(window)) {
        int width = 0;
        int height = 0;
        glfwGetWindowSize(window, &width, &height);

        if (width != prev_width || height != prev_height) {
            prev_width = width;
            prev_height = height;

            WGPUSwapChainDescriptor swapChainDes {
                .usage = WGPUTextureUsage_OUTPUT_ATTACHMENT,
                    .format = WGPUTextureFormat_Bgra8Unorm,
                    .width = static_cast<uint32_t>(width),
                    .height = static_cast<uint32_t>(height),
                    .present_mode = WGPUPresentMode_Fifo,
            };

            swap_chain = wgpu_device_create_swap_chain(device, surface, &swapChainDes);
        }

        WGPUSwapChainOutput next_texture = wgpu_swap_chain_get_next_texture(swap_chain);
        if (!next_texture.view_id) {
            printf("Cannot acquire next swap chain texture");
            return 1;
        }

        WGPUCommandBufferId cmdBuffers[2];

        // 1. Run compute shader

        WGPUCommandEncoderDescriptor computeCommandEncoderDescription {
            .label = "command encoder"
        };
        WGPUCommandEncoderId computeEncoder = wgpu_device_create_command_encoder(
            device, &computeCommandEncoderDescription
        );

        WGPUComputePassId computePassId = wgpu_command_encoder_begin_compute_pass(computeEncoder, NULL);
        wgpu_compute_pass_set_pipeline(computePassId, computePipeline);
        wgpu_compute_pass_set_bind_group(computePassId, 0, computeBingGroupId, NULL, 0);
        wgpu_compute_pass_dispatch(computePassId, 640 * 480, 1, 1);
        wgpu_compute_pass_end_pass(computePassId);

        cmdBuffers[0] = wgpu_command_encoder_finish(computeEncoder, NULL);

        // 2. Blit to swap chain

        WGPUCommandEncoderDescriptor commandEncorerDesc {
          .label = "command encoder"
        };

        WGPUCommandEncoderId cmd_encoder = wgpu_device_create_command_encoder(device, &commandEncorerDesc);

        WGPURenderPassColorAttachmentDescriptor
            color_attachments[ATTACHMENTS_LENGTH] = {
                {
                    .attachment = next_texture.view_id,
                    .load_op = WGPULoadOp_Clear,
                    .store_op = WGPUStoreOp_Store,
                    .clear_color = WGPUColor_GREEN,
                },
        };

        WGPURenderPassDescriptor renderPassDesc {
          .color_attachments = color_attachments,
          .color_attachments_length = RENDER_PASS_ATTACHMENTS_LENGTH,
          .depth_stencil_attachment = NULL,
        };

        WGPURenderPassId rpass = wgpu_command_encoder_begin_render_pass(cmd_encoder, &renderPassDesc);

        wgpu_render_pass_set_pipeline(rpass, render_pipeline);
        wgpu_render_pass_set_bind_group(rpass, 0, bindGroupId, NULL, 0);
        wgpu_render_pass_draw(rpass, 3, 1, 0, 0);
        wgpu_render_pass_end_pass(rpass);

        WGPUQueueId queue = wgpu_device_get_default_queue(device);

        cmdBuffers[1] = wgpu_command_encoder_finish(cmd_encoder, NULL);

        wgpu_queue_submit(queue, const_cast<const WGPUCommandBufferId*>(cmdBuffers), 2);

        wgpu_swap_chain_present(swap_chain);

        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
