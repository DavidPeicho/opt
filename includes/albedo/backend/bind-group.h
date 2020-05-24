#pragma once

#include <array>

#include <albedo/wgpu.h>

namespace albedo
{

namespace backend
{

// TODO: move to sources and use PIMPL.

template <uint8_t NbEntries>
class BindGroup
{
  public:

    BindGroup() noexcept;

    ~BindGroup() noexcept;

  public:

    template <typename T>
    void
    setEntry(T&& entry, uint8_t at);

    void
    create(WGPUDeviceId deviceId, WGPUBindGroupLayoutId bindGroupLayoutId);

    inline WGPUBindGroupId
    id() const { return m_id; }

  private:

    WGPUBindGroupId m_id;

    std::array<WGPUBindGroupEntry, NbEntries> m_entries;
    WGPUBindGroupDescriptor m_bindGroupDescriptor;

};

} // namespace backend

} // namespace albedo

#include <albedo/backend/bind-group.hxx>
