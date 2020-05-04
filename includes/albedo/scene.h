#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <albedo/instance.h>

namespace albedo
{

namespace
{

struct InstanceHash {

  size_t
  operator()(const Instance& i) const
  {
      return i.getId();
  }

  };

};

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
    Scene();

  public:

    Scene&
    addInstance(Instance instance, InstanceData&& = InstanceData{});

    Scene&
    deleteInstance(Instance instance);

  public:

    inline glm::mat4&
    transform(Instance instance)
    {
      // TODO: refactor this with the `data` method.

      auto pos = m_EntityToIndex.find(instance);
      if (pos != m_EntityToIndex.end())
      {
        return m_transforms[pos->second].modelToLocal;
      }

      // TODO: find a better way than throwin.
      throw std::string("invalid instance");
    }

    inline InstanceData&
    data(Instance instance)
    {
      auto pos = m_EntityToIndex.find(instance);
      if (pos != m_EntityToIndex.end())
      {
        return m_data[pos->second].second;
      }
      // TODO: find a better way than throwin.
      throw std::string("invalid instance");
    }

  private:
    std::unordered_map<Instance, Instance::Size, InstanceHash> m_EntityToIndex;

    std::vector<std::pair<Instance, InstanceData>> m_data;
    std::vector<Transform> m_transforms;
};

} // nanespace albedo
