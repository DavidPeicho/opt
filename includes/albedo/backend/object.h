#pragma once

#include <albedo/wgpu.h>

namespace albedo
{

namespace backend
{

// TODO: move to sources and use PIMPL.

template <typename Id>
class WGPUObject
{
  public:

    WGPUObject() noexcept;

    virtual ~WGPUObject() noexcept = 0;

  public:

    void create(WGPUDeviceId deviceId) = 0;

  public:

    inline Id
    id() const { return m_id; }

  private:

    Id m_id;
};

} // namespace backend

} // namespace albedo
