#pragma once

#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <albedo/scene.h>
#include <albedo/renderer.h>

#define TINYGLTF_NO_STB_IMAGE
#include <albedoloader/tiny_gltf.h>

namespace albedo
{

namespace loader
{

class GLTFLoader
{
  public:

    std::optional<Scene>
    load(Renderer& renderer, const std::string& path);

  private:

    void
    processMeshes(Scene& scene, const tinygltf::Model& model);

    void
    processNode(Scene& scene, const tinygltf::Node& node);

  private:

    std::vector<Scene::MeshPtr> m_meshes;

};

std::optional<Scene>
glTFLoadScene(Renderer& renderer, const std::string& path);

} // nanespace scene

} // nanespace albedo
