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

  inline typename std::vector<T>::iterator
  beginEntry(size_t i)
  {
    if (i >= entries.size()) { return data.end(); }
    return data.begin() + entries[i];
  }

  inline typename std::vector<T>::iterator
  endEntry(size_t i)
  {
    if (i + 1 >= entries.size()) { return data.end(); }
    return data.begin() + entries[i + 1];
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

// `mat4` structure aligment
// TODO: use
struct InstanceGPU
{
  glm::mat4 modelToWorld;
  uint32_t bvhRootIndex;
  uint32_t materialIndex;
  uint32_t padding_0;
  uint32_t padding_1;
};

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

    Scene&
    addMaterial(Material&& material);

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
    inline Material&
    getMaterial(size_t index)
    {
      return m_materials[index];
    }

    template <typename T>
    OptionalRef<T>
    data(const Entity&);

    inline TransformManager&
    transforms() { return m_transforms; }

  private:
    ComponentArray<InstanceData> m_data;
    ComponentArray<Renderable> m_renderables;

    TransformManager m_transforms;

    std::vector<Mesh::MeshPtr> m_meshes;

    std::vector<InstanceGPU> m_instances;
    std::vector<Material> m_materials;
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
