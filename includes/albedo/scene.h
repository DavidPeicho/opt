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

    Instance
    createInstance(InstanceData = InstanceData{});

    void
    deleteInstance();

  private:
    std::unordered_map<Instance, Instance::Size, InstanceHash> m_EntityToIndex;

    std::vector<InstanceData> m_data;
    std::vector<Transform> m_transforms;
};

} // nanespace albedo
