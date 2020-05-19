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
#include <albedo/resource-manager.h>

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

// TODO: template over `InstanceData` to accept extending that.
// TODO: add method to synchronize resources. What happens if the layout
// of the ResourceManager changes, we need to sync everything.
class Scene
{

  public:
    Scene(const ResourceManager::Ptr& manager);

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

    // TODO: expose a renderable manager directly.
    // TODO: improve API by letting user set the instanc with the mesh?
    Scene&
    addRenderable(Instance instance, size_t meshIndex);

    // TODO: expose a renderable manager directly.
    Scene&
    deleteRenderable(Instance instance);

  public:

    template <typename T>
    T&
    data(Instance instance);

  private:
    ComponentArray<InstanceData> m_data;
    ComponentArray<Transform> m_transforms;
    ComponentArray<Renderable> m_renderables;
    ComponentArray<Material> m_materials;

    ResourceManager::Ptr m_resourceManager;
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
inline Mesh::MeshPtr&
Scene::data<Mesh::MeshPtr>(Instance instance) { return m_meshes.data(instance); }

} // namespace albedo
