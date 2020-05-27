#pragma once

#include <list>
#include <memory>
#include <unordered_map>
#include <string>
#include <type_traits>
#include <vector>

#include <glm/glm.hpp>
#include <albedo/components/component-structure.h>
#include <albedo/instance.h>
#include <albedo/material.h>
#include <albedo/mesh.h>

namespace albedo
{

namespace
{
  template <typename T>
  struct FalseTrait : std::false_type {};
}

struct Transform
{
  glm::mat4 modelToLocal;
  glm::mat4 localToWorld;
};

struct InstanceData
{
  std::string name;
};

struct Renderable
{
  glm::mat4 modelToWorld;
  uint32_t bvhRootIndex;
  uint32_t materialIndex;
};

struct VertexGPU
{
  VertexGPU(const Vertex& vertex)
    : position{vertex.position}
  { }

  glm::vec3 position;
  uint32_t padding0;
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

// TODO: template over `InstanceData` to accept extending that.
// TODO: add method to synchronize resources. What happens if the layout
// of the ResourceManager changes, we need to sync everything.
class Scene
{

  friend class Renderer;

  public:
    Scene() noexcept = default;

    Scene(Scene&&) noexcept = default;

    Scene(const Scene&) = delete;

    Scene&
    operator=(const Scene&) = delete;

  public:

    Scene&
    addInstance(Instance instance, InstanceData&& = InstanceData{});

    Scene&
    deleteInstance(Instance instance);

  public:

    Scene&
    addMesh(const Mesh::MeshPtr& mesh);

    Scene&
    addMeshes(const std::vector<Mesh::MeshPtr>& meshes);

    // TODO: expose a renderable manager directly.
    // TODO: improve API by letting user set the instanc with the mesh?
    Scene&
    addRenderable(Instance instance, size_t meshIndex);

    // TODO: expose a renderable manager directly.
    Scene&
    deleteRenderable(Instance instance);

    void
    build(); // TODO: improve this unclear API point.

  public:

    template <typename T>
    T&
    data(Instance instance);

  private:
    ComponentArray<InstanceData> m_data;
    ComponentArray<Transform> m_transforms;
    ComponentArray<Renderable> m_renderables;
    ComponentArray<Material> m_materials;

    std::vector<Mesh::MeshPtr> m_meshes;
    std::vector<Vertex> m_vertices; // Vertices of **all** BVH.
    Mesh::IndexBuffer m_indices; // Indices of **all** BVH.
    std::vector<BVHNodeGPU> m_nodes; // Nodes of all BVH.
};

template <typename T>
T&
Scene::data(Instance instance)
{
  static_assert(FalseTrait<T>::value, "unsupported type. Check available components");
}

template <>
inline InstanceData&
Scene::data<InstanceData>(Instance instance) { return m_data.data(instance); }

template <>
inline Transform&
Scene::data<Transform>(Instance instance) { return m_transforms.data(instance); }

template <>
inline Renderable&
Scene::data<Renderable>(Instance instance) { return m_renderables.data(instance); }

} // namespace albedo
