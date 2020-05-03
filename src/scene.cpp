#include <albedo/scene.h>

namespace albedo
{

namespace
{

  template <typename T>
  inline void
  deleteAndShift(std::vector<T>& vec, size_t index)
  {
    // The vector is assumed to always have at least one element.
    vec[index] = std::move(vec[vec.size() - 1]);
    vec.pop_back();
  }

}

Scene::Scene()
{

}

Scene&
Scene::addInstance(Instance instance, InstanceData&& data)
{
  // TODO: add test to see if instance is already in the scene.
  // Only check in debug mode.

  // TODO: either make this function thread safe, or ask the user to make a
  // thread safe call.

  Instance::Size index = m_data.size();

  m_data.push_back(std::make_pair(instance, std::move(data)));
  m_transforms.push_back(Transform{});
  m_EntityToIndex[instance] = index;

  return *this;
}

Scene&
Scene::deleteInstance(Instance instance)
{
  // TODO: either make this function thread safe, or ask the user to make a
  // thread safe call.

  auto instanceIt = m_EntityToIndex.find(instance);
  if (instanceIt == m_EntityToIndex.end()) { return *this; }

  // Index at which the data of this instance is located in every data buffer:
  // transforms, etc...
  auto dataIndex = instanceIt->second;

  // Instance associated to the last data entry. This will be moved at the
  // position of the deleted instance.
  auto lastInstance = m_data[m_data.size() - 1].first;

  // Move the last element of every array at the position pointed by the
  // instance we are currently removing.
  deleteAndShift(m_data, dataIndex);
  deleteAndShift(m_transforms, dataIndex);

  // Erase the target instance from the index map, and update the index of
  // the previous last element.
  m_EntityToIndex.erase(instanceIt);
  if (instance != lastInstance) { m_EntityToIndex[lastInstance] = dataIndex; }

  return *this;
}

} // nanespace albedo
