/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <stdio.h>
#include <iostream>
#include <stdlib.h>

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

#include <albedoloader/gltf-loader.h>
#include <albedo/accel/gpu-accel.h>
#include <albedo/scene.h>

int main() {
    albedo::Renderer renderer;
    albedo::loader::GLTFLoader loader;
    auto scene = loader.load(renderer, "../box.glb");
    if (!scene)
    {
      std::cerr << "Failed to load scene" << std::endl;
      return 1;
    }

    albedo::accel::GPUAccelerationStructure accelerationStructure;
    accelerationStructure.addMeshes(loader.meshes());
    accelerationStructure.build(scene.value());

    return 0;

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

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
