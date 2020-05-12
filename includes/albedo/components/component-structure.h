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

  template <typename T>
  class ComponentArray
  {
    public:

      void
      add(Instance instance, T&& data);

      void
      remove(Instance instance);

      inline T&
      data(Instance instance)
      {
        auto pos = m_entityToIndex.find(instance);
        if (pos != m_entityToIndex.end())
        {
          return m_data[pos->second];
        }
        // TODO: find a better way than throwin.
        throw std::string("invalid instance");
      }

      inline const std::vector<T>&
      all() const { m_data; }

    private:

      std::unordered_map<Instance, Instance::Size, InstanceHash> m_entityToIndex;
      std::vector<Instance> m_instances;
      std::vector<T> m_data;

  };

}

#include "component-structure.hxx"
