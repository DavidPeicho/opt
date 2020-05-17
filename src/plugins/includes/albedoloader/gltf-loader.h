#pragma once

#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <albedo/mesh.h>
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

  public:

    inline std::vector<Mesh::MeshPtr>&
    meshes() { return m_meshes; }

  private:

    void
    processMeshes(Scene& scene, const tinygltf::Model& model);

    void
    processNode(Scene& scene, const tinygltf::Node& node, const tinygltf::Model& model);

  private:

    std::vector<Mesh::MeshPtr> m_meshes;

};

std::optional<Scene>
glTFLoadScene(Renderer& renderer, const std::string& path);

} // nanespace scene

} // nanespace albedo
