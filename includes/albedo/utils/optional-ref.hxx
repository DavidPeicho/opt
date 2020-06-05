#pragma once

namespace albedo
{

template <class T>
OptionalRef<T>::OptionalRef() noexcept
  : m_ref{std::nullopt}
{ }

template <class T>
OptionalRef<T>::OptionalRef(T& value) noexcept
  : m_ref{value}
{ }

template <class T>
constexpr T&
OptionalRef<T>::value()
{
  return m_ref.value();
}

template <class T>
constexpr const T&
OptionalRef<T>::value() const
{
  return m_ref.value();
}

template <class T>
constexpr bool
OptionalRef<T>::has_value() const noexcept
{
  return m_ref.has_value();
}

template <class T>
constexpr
OptionalRef<T>::operator bool() const noexcept
{
  return m_ref.has_value();
}

template <class T>
constexpr const T*
OptionalRef<T>::operator->() const
{
  return &m_ref.value().get();
}

template <class T>
constexpr T*
OptionalRef<T>::operator->()
{
  return &m_ref.value().get();
}

template <class T>
constexpr const T&
OptionalRef<T>::operator*() const
{
  return m_ref.value().get();
}

template <class T>
constexpr T&
OptionalRef<T>::operator*()
{
  return m_ref.value().get();
}

} // namespace albedo
