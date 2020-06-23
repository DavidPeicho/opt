#pragma once

#include <albedo/wgpu.h>

namespace albedo
{

namespace backend
{

// TODO: move to sources and use PIMPL.

template <typename Descriptor>
class WGPUObject
{
  public:

    WGPUObject() noexcept = default;

    virtual ~WGPUObject() noexcept
    { }

  public:

    bool
    operator==(const WGPUObject& other) const { return other.m_id == m_id; }

    bool
    operator!=(const WGPUObject& other) const { return other.m_id != m_id; }

  public:

    inline void
    setDescriptor(const Descriptor& descriptor) { m_descriptor = descriptor; }

    inline WGPUNonZeroU64
    id() const { return m_id; }

  protected:

    WGPUNonZeroU64 m_id;
    Descriptor m_descriptor;
};

} // namespace backend

} // namespace albedo
