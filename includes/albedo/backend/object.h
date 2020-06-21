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

    virtual ~WGPUObject() noexcept = 0;

  public:

    virtual bool
    operator==(const WGPUObject& other) const { return other.m_id == m_id; }

    virtual bool
    operator!=(const WGPUObject& other) const { return other.m_id != m_id; }

  public:

    inline WGPUNonZeroU64
    id() const { return m_id; }

  protected:

    WGPUNonZeroU64 m_id;
    Descriptor m_descriptor;
};

} // namespace backend

} // namespace albedo
