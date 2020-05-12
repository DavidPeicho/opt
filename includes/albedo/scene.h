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

// TODO: template over `InstanceData` to accept extending that.
class Scene
{

  public:
    using MeshPtr = std::shared_ptr<Mesh>;

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
    addMesh(Instance instance, Mesh&& mesh);

    Scene&
    addMesh(Instance instance, MeshPtr& meshPtr);

    Scene&
    removeMesh(Instance instance);

  public:

    template <typename T>
    T&
    data(Instance instance);

    const ComponentArray<MeshPtr>&
    meshes() const { return m_meshes; }

  private:
    ComponentArray<InstanceData> m_data;
    ComponentArray<Transform> m_transforms;
    ComponentArray<MeshPtr> m_meshes;
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
inline Scene::MeshPtr&
Scene::data<Scene::MeshPtr>(Instance instance) { return m_meshes.data(instance); }

} // namespace albedo
