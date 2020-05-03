#pragma once

#include <atomic>
#include <cstddef>

namespace albedo
{

class Instance {

  public:
    using Size = std::size_t;

  public:

    Instance() noexcept
      : m_ID(++globalId)
    { }

    size_t
    operator()() const {
      return m_ID;
    }

    bool
    operator==(Instance e) const { return e.m_ID == m_ID; }

    bool
    operator!=(Instance e) const { return e.m_ID != m_ID; }

    bool
    operator<(Instance e) const { return e.m_ID < m_ID; }

    // an id that can be used for debugging/printing
    inline Size
    getId() const noexcept { return m_ID; }

  private:
    Size m_ID = 0;
    static std::atomic<Instance::Size> globalId;
};

}
