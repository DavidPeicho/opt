#pragma once

#include <unordered_set>
#include <vector>

#include <albedo/instance.h>

namespace albedo
{

  struct InstanceHash {

    size_t
    operator()(const Instance& i) const
    {
        return i.getId();
    }

  };

  // TODO: use a structure of array here instead.
  template <typename T>
  class ComponentArray
  {
    public:

      void
      add(Instance instance, T&& data);

      void
      remove(Instance instance);

      inline std::optional<std::reference_wrapper<T>>
      data(Instance instance)
      {
        auto pos = m_entityToIndex.find(instance);
        if (pos != m_entityToIndex.end())
        {
          return m_data[pos->second];
        }
        return std::nullopt;
      }

      inline const std::vector<T>&
      all() const { return m_data; }

      inline const std::vector<Instance>&
      instances() const { return m_instances; }

    private:

      std::unordered_map<Instance, Instance::Size, InstanceHash> m_entityToIndex;
      std::vector<Instance> m_instances;
      std::vector<T> m_data;

  };

}

#include "component-structure.hxx"
