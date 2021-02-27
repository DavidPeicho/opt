#pragma once

#include <list>
#include <memory>
#include <unordered_map>
#include <string>
#include <type_traits>
#include <vector>

#include <glm/glm.hpp>
#include <albedo/components/component.h>
#include <albedo/components/component-structure.h>
#include <albedo/components/material.h>
#include <albedo/components/light.h>
#include <albedo/components/transform.h>
#include <albedo/entity.h>
#include <albedo/components/material.h>
#include <albedo/mesh.h>
#include <albedo/utils/index.h>
#include <albedo/utils/optional-ref.h>

namespace albedo
{

namespace debug
{
  class SceneDebugger;
}

using TransformManager = components::TransformManager;

namespace
{
  template <typename T>
  struct FalseTrait : std::false_type {};
}

struct InstanceData: public Component<InstanceData>
{
  std::string name;
};

struct Renderable: public Component<Renderable>
{
  Mesh::IndexType meshIndex;
  Mesh::IndexType materialIndex;
};

// TODO: Separate leaf from internal nodes.
struct BVHNodeGPU
{
  using NodeList = std::vector<BVHNodeGPU>;

  glm::vec3 min;
  uint32_t nextNodeIndex;
  glm::vec3 max;
  uint32_t primitiveStartIndex;
};

// `mat4` structure aligment
// TODO: use
struct InstanceGPU
{
  glm::mat4 worldToModel;
  uint32_t bvhRootIndex;
  uint32_t materialIndex;
  uint32_t padding_0;
  uint32_t padding_1;
};

// Origin saved in normal.z, tangent.z, bitangent.z
struct LightGPU
{
  glm::vec4 normal;
  glm::vec4 tangent;
  glm::vec4 bitangent;
  float intensity;
  float padding_0;
  float padding_1;
  float padding_2;
};

struct TexturesInfo
{
  std::vector<std::vector<unsigned char>> textures;
  uint width;
  uint height;
};

// TODO: add method to synchronize resources. What happens if the layout
// of the ResourceManager changes, we need to sync everything.
class Scene
{
  friend class Renderer;
  friend class debug::SceneDebugger;

  public:
    Scene() noexcept = default;

    Scene(Scene&&) noexcept = default;

    Scene(const Scene&) = delete;

    Scene&
    operator=(const Scene&) = delete;

  public:

    Scene&
    addInstance(const Entity&, InstanceData&& = InstanceData{});

    Scene&
    deleteInstance(const Entity&);

  public:

    Scene&
    addMesh(const Mesh::MeshPtr& mesh);

    Scene&
    addMeshes(const std::vector<Mesh::MeshPtr>& meshes);

    Scene&
    addMaterial(components::Material&& material);

    Scene&
    setTexturesInfo(TexturesInfo&& info);

    // TODO: remove material.

    // TODO: expose a renderable manager directly.
    // TODO: improve API by letting user set the instanc with the mesh?
    // TODO: improve the API with the material index
    Scene&
    addRenderable(const Entity&, size_t meshIndex, size_t materialIndex);

    // TODO: expose a renderable manager directly.
    Scene&
    deleteRenderable(const Entity&);

    Scene&
    update();

    void
    build(); // TODO: improve this unclear API point.

  public:

    // TODO: improve material API.
    // Right now, it's hard to use, and hard not to make mistakes...
    // When removing a material what happens to the entity pointing to it?
    inline components::Material&
    getMaterial(size_t index)
    {
      return m_materials[index];
    }

    template <typename T>
    OptionalRef<T>
    data(const Entity&);

    inline components::LightManager&
    lights() { return m_lightsManager; }

    inline TransformManager&
    transforms() { return m_transforms; }

  private:
    ComponentArray<InstanceData> m_data;
    ComponentArray<Renderable> m_renderables;

    components::LightManager m_lightsManager;
    TransformManager m_transforms;

    std::vector<Mesh::MeshPtr> m_meshes;
    TexturesInfo m_texturesInfo;

    std::vector<InstanceGPU> m_instances;
    std::vector<components::Material> m_materials;
    std::vector<LightGPU> m_lights;
    std::vector<Vertex> m_vertices; // Vertices of **all** BVH.
    std::vector<Mesh::IndexType> m_indices; // Indices of **all** BVH.
    std::vector<BVHNodeGPU> m_nodes; // Nodes of all BVH.
};

template <typename T>
OptionalRef<T>
Scene::data(const Entity& entity)
{
  static_assert(FalseTrait<T>::value, "unsupported type. Check available components");
}

template <>
inline OptionalRef<InstanceData>
Scene::data<InstanceData>(const Entity& entity)
{
  return m_data.getComponentData(entity);
}

template <>
inline OptionalRef<Renderable>
Scene::data<Renderable>(const Entity& entity)
{
  return m_renderables.getComponentData(entity);
}

} // namespace albedo
