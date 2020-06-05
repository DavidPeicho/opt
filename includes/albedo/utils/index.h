#pragma once

#include <atomic>
#include <cstddef>
#include <limits>

namespace albedo
{

class Index {

  public:

    using Size = std::size_t;

  public:

    Index() noexcept = default;
    Index(const Index& e) noexcept = default;
    Index(Index&& e) noexcept = default;

    Index&
    Index=(const Index& e) noexcept = default;

    Index&
    Index=(Index&& e) noexcept = default;

    size_t
    operator()() const { return m_value; }

    bool
    operator==(Index e) const { return e.m_value == m_value; }

    bool
    operator!=(Index e) const { return e.m_value != m_value; }

    bool
    operator<(Index e) const { return e.m_value < m_value; }

    inline bool
    isDestroyed()
    {
      return m_value == std::numeric_limits<Index::Size>::max();
    }

    // an id that can be used for debugging/printing
    inline Size
    getValue() const noexcept { return m_value; }

  protected:

    Size m_value = 0;

};

// TODO: using a global counter like that isn't a good idea, as we may run
// out of values in the future. A class managing alive / dead entities could
// help with that.
class GlobalIdentifier: Index
{

  public:

    GlobalIdentifier() noexcept
      : Index(++globalId)
    { }

  private:

    // TODO: generate randomly instead.
    static std::atomic<Index::Size> globalId;

}

}
