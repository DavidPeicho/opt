#pragma once

#include <list>
#include <memory>
#include <unordered_map>
#include <string>
#include <type_traits>
#include <vector>

#include <glm/glm.hpp>
#include <albedo/components/component-structure.h>
#include <albedo/components/transform.h>
#include <albedo/instance.h>
#include <albedo/material.h>
#include <albedo/mesh.h>

namespace albedo
{

using TransformManager = components::TransformManager;

namespace
{
  template <typename T>
  struct FalseTrait : std::false_type {};
}

struct InstanceData
{
  std::string name;
};

struct Renderable
{
  Mesh::IndexType meshIndex;
  Mesh::IndexType materialIndex;
};

// TODO: move out of Scene.
// TODO: improve this class API. Right now it wraps stuff without much reasons,
// even though it does memoization of start indices.
template <class T>
struct EntryOffsetTable
{
  inline void
  push(const std::vector<T>& entry)
  {
    auto start = data.size();
    data.insert(data.end(), entry.begin(), entry.end());
    entries.push_back(start);
  }

  inline void
  clear()
  {
    data.clear();
    entries.clear();
  }

  std::vector<T> data;
  std::vector<uint32_t> entries;

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

struct InstanceGPU
{
  glm::mat4 modelToWorld;
  uint32_t bvhRootIndex;
  uint32_t materialIndex;
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

    Scene&
    update();

    void
    build(); // TODO: improve this unclear API point.

  public:

    // TODO: add perfect forwarding to m_data, and put this method const.
    std::vector<Instance>
    findByName(const std::string& name);

  public:

    template <typename T>
    T&
    data(Instance instance);

    inline TransformManager&
    getTransformManager() { return m_transformManager; }

  private:
    ComponentArray<InstanceData> m_data;
    ComponentArray<Renderable> m_renderables;
    ComponentArray<Material> m_materials;

    TransformManager m_transformManager;

    std::vector<Mesh::MeshPtr> m_meshes;
    std::vector<InstanceGPU> m_instances;

    EntryOffsetTable<Vertex> m_vertices; // Vertices of **all** BVH.
    EntryOffsetTable<Mesh::IndexType> m_indices; // Indices of **all** BVH.
    EntryOffsetTable<BVHNodeGPU> m_nodes; // Nodes of all BVH.
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
inline Renderable&
Scene::data<Renderable>(Instance instance) { return m_renderables.data(instance); }

} // namespace albedo
