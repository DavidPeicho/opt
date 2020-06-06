#pragma once

#include <optional>
#include <unordered_set>
#include <vector>

#include <albedo/entity.h>
#include <albedo/utils/index.h>
#include <albedo/utils/optional-ref.h>

namespace albedo
{

struct EntityHash
{

  size_t
  operator()(const Entity& i) const
  {
      return i.getValue();
  }

};

// TODO: use a structure of array here instead.
// TODO: implement iterator functions
template <typename DataType>
class ComponentArray
{

  public:

    using EntityContainer = std::vector<Entity>;
    using DataContainer = std::vector<DataType>;

  public:

    void
    createComponent(const Entity& entity, DataType&& data);

    void
    removeComponent(const Entity& entity);

    bool
    hasComponent(const Entity& entity);

    std::optional<typename DataType::Id>
    getComponent(const Entity& entity) const;

    OptionalRef<DataType>
    getComponentData(const Entity& entity);

    OptionalRef<const DataType>
    getComponentData(const Entity& entity) const;

    inline const DataContainer&
    components() const { return m_data; }

    inline const EntityContainer&
    entities() const { return m_entities; }

  private:

    std::unordered_map<Entity, typename DataType::Id, EntityHash> m_entityToIndex;
    EntityContainer m_entities;
    DataContainer m_data;

};

}

#include "component-structure.hxx"
