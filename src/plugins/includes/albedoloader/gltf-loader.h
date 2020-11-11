#pragma once

#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <albedo/entity.h>
#include <albedo/mesh.h>
#include <albedo/scene.h>

#include <albedoloader/tiny_gltf.h>

namespace albedo
{

namespace loader
{

class GLTFLoader
{
  public:

    std::optional<Scene>
    load(const std::string& path);

  public:

    inline std::vector<Mesh::MeshPtr>&
    meshes() { return m_meshes; }

  private:

    void
    processMaterials(Scene& scene, const tinygltf::Model& model);

    void
    processMeshes(Scene& scene, const tinygltf::Model& model);

    Entity
    processNode(Scene& scene, const tinygltf::Node& node, const tinygltf::Model& model);

  private:

    std::vector<Mesh::MeshPtr> m_meshes;

};

} // nanespace scene

} // nanespace albedo
