#pragma once

#include <optional>
#include <unordered_set>
#include <vector>

#include <albedo/entity.h>
#include <albedo/utils/identifier.h>

namespace albedo
{

struct EntityHash
{

  size_t
  operator()(const Entity& i) const
  {
      return i.getId();
  }

};

// TODO: use a structure of array here instead.
template <typename Instance, typename DataType>
class ComponentArray
{
  public:

    Instance
    createComponent(Entity entity, T&& data);

    void
    removeComponent(Instance instance);

    bool
    hasComponent(const Entity& entity);

    std::optional<Instance>
    getComponent(const Entity& entity);

    inline DataType&
    data(const Instance& component);

    inline const DataType&
    data(const Instance& ComponentInstance) const;

    inline OptionalRef<T>
    getComponentData(Instance instance)
    {
      auto pos = m_entityToIndex.find(instance);
      if (pos != m_entityToIndex.end())
      {
        return OptionalRef{m_data[pos->second]};
      }
      return OptionalRef{};
    }

    inline const std::vector<T>&
    all() const { return m_data; }

    inline const std::vector<Instance>&
    instances() const { return m_instances; }

  private:

    std::unordered_map<Entity, Identifier::Size, EntityHash> m_entityToIndex;
    std::vector<Instance> m_instances;
    std::vector<DataType> m_data;

};

}

#include "component-structure.hxx"
