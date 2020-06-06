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
#include <albedo/components/transform.h>
#include <albedo/entity.h>
#include <albedo/material.h>
#include <albedo/mesh.h>
#include <albedo/utils/index.h>
#include <albedo/utils/optional-ref.h>

namespace albedo
{

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
    addInstance(const Entity&, InstanceData&& = InstanceData{});

    Scene&
    deleteInstance(const Entity&);

  public:

    Scene&
    addMesh(const Mesh::MeshPtr& mesh);

    Scene&
    addMeshes(const std::vector<Mesh::MeshPtr>& meshes);

    // TODO: expose a renderable manager directly.
    // TODO: improve API by letting user set the instanc with the mesh?
    Scene&
    addRenderable(const Entity&, size_t meshIndex);

    // TODO: expose a renderable manager directly.
    Scene&
    deleteRenderable(const Entity&);

    Scene&
    update();

    void
    build(); // TODO: improve this unclear API point.

  public:

    template <typename T>
    OptionalRef<T>
    data(const Entity&);

    inline TransformManager&
    getTransformManager() { return m_transformManager; }

  private:
    ComponentArray<InstanceData> m_data;
    ComponentArray<Renderable> m_renderables;

    TransformManager m_transformManager;

    std::vector<Mesh::MeshPtr> m_meshes;
    std::vector<InstanceGPU> m_instances;

    EntryOffsetTable<Vertex> m_vertices; // Vertices of **all** BVH.
    EntryOffsetTable<Mesh::IndexType> m_indices; // Indices of **all** BVH.
    EntryOffsetTable<BVHNodeGPU> m_nodes; // Nodes of all BVH.
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
