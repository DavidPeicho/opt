#pragma once

#include <atomic>
#include <cstddef>
#include <limits>

namespace albedo
{

class Index {

  public:

    using Type = std::size_t;

  public:

    explicit Index(Index::Type value) noexcept
      : m_value(value)
    { }
    Index(const Index& e) noexcept = default;
    Index(Index&& e) noexcept = default;

    Index&
    operator=(const Index& e) noexcept = default;

    Index&
    operator=(Index&& e) noexcept = default;

    size_t
    operator()() const { return m_value; }

    constexpr operator Type() const noexcept { return m_value; }

    bool
    operator==(Index e) const { return e.m_value == m_value; }

    bool
    operator!=(Index e) const { return e.m_value != m_value; }

    bool
    operator<(Index e) const { return e.m_value < m_value; }

    inline bool
    isValid()
    {
      return m_value != std::numeric_limits<Index::Type>::max();
    }

    // an id that can be used for debugging/printing
    inline Type
    getValue() const noexcept { return m_value; }

  protected:

    Type m_value = 0;

};

/* Overload comparisons of a `Index` object with `Index::Type`. */

constexpr bool
operator==(const Index& i, Index::Type rhs) noexcept
{
  return i.getValue() == rhs;
}

constexpr bool
operator==(Index::Type lhs, const Index& i) noexcept
{
  return i.getValue() == lhs;
}

constexpr bool
operator!=(const Index& i, Index::Type rhs) noexcept
{
  return i.getValue() != rhs;
}

constexpr bool
operator!=(Index::Type lhs, const Index& i) noexcept
{
  return i.getValue() != lhs;
}

constexpr bool
operator<(const Index& i, Index::Type rhs) noexcept
{
  return i.getValue() < rhs;
}

constexpr bool
operator<(Index::Type lhs , const Index& i) noexcept
{
  return i.getValue() < lhs;
}

constexpr bool
operator<=(const Index& i, Index::Type rhs) noexcept
{
  return i.getValue() <= rhs;
}

constexpr bool
operator<=(Index::Type lhs, const Index& i) noexcept
{
  return i.getValue() <= lhs;
}

constexpr bool
operator>(const Index& i, Index::Type rhs) noexcept
{
  return i.getValue() > rhs;
}

constexpr bool
operator>(Index::Type lhs, const Index& i) noexcept
{
  return i.getValue() > lhs;
}

constexpr bool
operator>=(const Index& i, Index::Type rhs) noexcept
{
  return i.getValue() >= rhs;
}

constexpr bool
operator>=(Index::Type lhs, const Index& i) noexcept
{
  return i.getValue() >= lhs;
}

// TODO: using a global counter like that isn't a good idea, as we may run
// out of values in the future. A class managing alive / dead entities could
// help with that.
class GlobalIdentifier: public Index
{

  public:

    GlobalIdentifier() noexcept
      : Index(++globalId)
    { }

  private:

    // TODO: generate randomly instead.
    static std::atomic<Index::Type> globalId;

};

}
