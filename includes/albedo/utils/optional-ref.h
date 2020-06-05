#pragma once

#include <optional>

namespace albedo
{

template <class T>
class OptionalRef
{

  public:

    // TODO: add perfect forwarding

    OptionalRef() noexcept;

    OptionalRef(T& value) noexcept;

  public:

    constexpr T&
    value();

    constexpr const T&
    value() const;

    constexpr bool
    has_value() const noexcept;

  public:

    constexpr const T*
    operator->() const;

    constexpr T*
    operator->();

    constexpr const T&
    operator*() const;

    constexpr T&
    operator*();

    constexpr explicit
    operator bool() const noexcept;

  private:

    std::optional<std::reference_wrapper<T>> m_ref;

};

/* Overload comparisons of two `OptionalRef` object. */

template<class T, class U>
constexpr bool
operator==(const OptionalRef<T>& lhs, const OptionalRef<U>& rhs)
{
  return lhs.m_ref == rhs.m_ref;
}

template<class T, class U>
constexpr bool
operator!=(const OptionalRef<T>& lhs, const OptionalRef<U>& rhs)
{
  return lhs.m_ref != rhs.m_ref;
}

template<class T, class U>
constexpr bool
operator<(const OptionalRef<T>& lhs, const OptionalRef<U>& rhs)
{
  return lhs.m_ref < rhs.m_ref;
}

template<class T, class U>
constexpr bool
operator<=(const OptionalRef<T>& lhs, const OptionalRef<U>& rhs )
{
  return lhs.m_ref <= rhs.m_ref;
}

template<class T, class U>
constexpr bool
operator>(const OptionalRef<T>& lhs, const OptionalRef<U>& rhs)
{
  return lhs.m_ref > rhs.m_ref;
}

template<class T, class U>
constexpr bool
operator>=(const OptionalRef<T>& lhs, const OptionalRef<U>& rhs)
{
  return lhs.m_ref >= rhs.m_ref;
}

/* Overload comparisons of a `OptionalRef` object with `std::nullopt_t`. */

template<class T>
constexpr bool
operator==(const OptionalRef<T>& opt, std::nullopt_t rhs) noexcept
{
  return opt.m_ref == rhs;
}

template<class T>
constexpr bool
operator==(std::nullopt_t lhs, const OptionalRef<T>& opt) noexcept
{
  return opt.m_ref == lhs;
}

template<class T>
constexpr bool
operator!=(const OptionalRef<T>& opt, std::nullopt_t rhs) noexcept
{
  return opt.m_ref != rhs;
}

template<class T>
constexpr bool
operator!=(std::nullopt_t lhs, const OptionalRef<T>& opt) noexcept
{
  return opt.m_ref != lhs;
}

template<class T>
constexpr bool
operator<(const OptionalRef<T>& opt, std::nullopt_t rhs) noexcept
{
  return opt.m_ref < rhs;
}

template<class T>
constexpr bool
operator<(std::nullopt_t lhs , const OptionalRef<T>& opt) noexcept
{
  return opt.m_ref < lhs;
}

template<class T>
constexpr bool
operator<=(const OptionalRef<T>& opt, std::nullopt_t rhs) noexcept
{
  return opt.m_ref <= rhs;
}

template<class T>
constexpr bool
operator<=(std::nullopt_t lhs, const OptionalRef<T>& opt) noexcept
{
  return opt.m_ref <= lhs;
}

template<class T>
constexpr bool
operator>(const OptionalRef<T>& opt, std::nullopt_t rhs) noexcept
{
  return opt.m_ref > rhs;
}

template<class T>
constexpr bool
operator>(std::nullopt_t lhs, const OptionalRef<T>& opt) noexcept
{
  return opt.m_ref > lhs;
}

template<class T>
constexpr bool
operator>=(const OptionalRef<T>& opt, std::nullopt_t rhs) noexcept
{
  return opt.m_ref >= rhs;
}

template<class T>
constexpr bool
operator>=(std::nullopt_t lhs, const OptionalRef<T>& opt) noexcept
{
  return opt.m_ref >= lhs;
}

/* Overload comparisons of a `OptionalRef` object with object `T`. */

template<class T, class U>
constexpr bool
operator==(const OptionalRef<T>& opt, const U& rhs) noexcept
{
  return opt.m_ref == rhs;
}

template<class T, class U>
constexpr bool
operator==(const U& lhs, const OptionalRef<T>& opt) noexcept
{
  return opt.m_ref == lhs;
}

template<class T, class U>
constexpr bool
operator!=(const OptionalRef<T>& opt, const U& rhs) noexcept
{
  return opt.m_ref != rhs;
}

template<class T, class U>
constexpr bool
operator!=(const U& lhs, const OptionalRef<T>& opt) noexcept
{
  return opt.m_ref != lhs;
}

template<class T, class U>
constexpr bool
operator<(const OptionalRef<T>& opt, const U& rhs) noexcept
{
  return opt.m_ref < rhs;
}

template<class T, class U>
constexpr bool
operator<(const U& lhs , const OptionalRef<T>& opt) noexcept
{
  return opt.m_ref < lhs;
}

template<class T, class U>
constexpr bool
operator<=(const OptionalRef<T>& opt, const U& rhs) noexcept
{
  return opt.m_ref <= rhs;
}

template<class T, class U>
constexpr bool
operator<=(const U& lhs, const OptionalRef<T>& opt) noexcept
{
  return opt.m_ref <= lhs;
}

template<class T, class U>
constexpr bool
operator>(const OptionalRef<T>& opt, const U& rhs) noexcept
{
  return opt.m_ref > rhs;
}

template<class T, class U>
constexpr bool
operator>(const U& lhs, const OptionalRef<T>& opt) noexcept
{
  return opt.m_ref > lhs;
}

template<class T, class U>
constexpr bool
operator>=(const OptionalRef<T>& opt, const U& rhs) noexcept
{
  return opt.m_ref >= rhs;
}

template<class T, class U>
constexpr bool
operator>=(const U& lhs, const OptionalRef<T>& opt) noexcept
{
  return opt.m_ref >= lhs;
}

} // namespace albedo

#include <albedo/utils/optional-ref.hxx>
