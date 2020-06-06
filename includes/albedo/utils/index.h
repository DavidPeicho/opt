#pragma once

#include <atomic>
#include <cstddef>
#include <limits>

namespace albedo
{

template <typename Tag>
class Index {

  public:

    using Type = int;

  public:

    Index(Index::Type value) noexcept
      : m_value(value)
    { }

    Index() noexcept = default;

    Index(const Index& e) noexcept = default;
    Index(Index&& e) noexcept = default;

    Index&
    operator=(const Index& e) noexcept = default;

    Index&
    operator=(Index&& e) noexcept = default;

    Type
    operator()() const { return m_value; }

    constexpr operator Type() const noexcept { return m_value; }

    bool
    operator==(Index e) const { return e.m_value == m_value; }

    bool
    operator!=(Index e) const { return e.m_value != m_value; }

    bool
    operator<(Index e) const { return e.m_value < m_value; }

    // an id that can be used for debugging/printing
    inline Type
    getValue() const noexcept { return m_value; }

  protected:

    Type m_value = 0;

};

/* Overload comparisons of a `Index` object with `Index::Type`. */

template <typename Tag>
constexpr bool
operator==(const Index<Tag>& i, typename Index<Tag>::Type rhs) noexcept
{
  return i.getValue() == rhs;
}

template <typename Tag>
constexpr bool
operator==(typename Index<Tag>::Type lhs, const Index<Tag>& i) noexcept
{
  return i.getValue() == lhs;
}

template <typename Tag>
constexpr bool
operator!=(const Index<Tag>& i, typename Index<Tag>::Type rhs) noexcept
{
  return i.getValue() != rhs;
}

template <typename Tag>
constexpr bool
operator!=(typename Index<Tag>::Type lhs, const Index<Tag>& i) noexcept
{
  return i.getValue() != lhs;
}

template <typename Tag>
constexpr bool
operator<(const Index<Tag>& i, typename Index<Tag>::Type rhs) noexcept
{
  return i.getValue() < rhs;
}

template <typename Tag>
constexpr bool
operator<(typename Index<Tag>::Type lhs , const Index<Tag>& i) noexcept
{
  return i.getValue() < lhs;
}

template <typename Tag>
constexpr bool
operator<=(const Index<Tag>& i, typename Index<Tag>::Type rhs) noexcept
{
  return i.getValue() <= rhs;
}

template <typename Tag>
constexpr bool
operator<=(typename Index<Tag>::Type lhs, const Index<Tag>& i) noexcept
{
  return i.getValue() <= lhs;
}

template <typename Tag>
constexpr bool
operator>(const Index<Tag>& i, typename Index<Tag>::Type rhs) noexcept
{
  return i.getValue() > rhs;
}

template <typename Tag>
constexpr bool
operator>(typename Index<Tag>::Type lhs, const Index<Tag>& i) noexcept
{
  return i.getValue() > lhs;
}

template <typename Tag>
constexpr bool
operator>=(const Index<Tag>& i, typename Index<Tag>::Type rhs) noexcept
{
  return i.getValue() >= rhs;
}

template <typename Tag>
constexpr bool
operator>=(typename Index<Tag>::Type lhs, const Index<Tag>& i) noexcept
{
  return i.getValue() >= lhs;
}

// TODO: using a global counter like that isn't a good idea, as we may run
// out of values in the future. A class managing alive / dead entities could
// help with that.
class GlobalIdentifier: public Index<GlobalIdentifier>
{

  public:

    GlobalIdentifier() noexcept
      : Index(++globalId)
    { }

  private:

    // TODO: generate randomly instead.
    static std::atomic<Index<GlobalIdentifier>::Type> globalId;

};

}
