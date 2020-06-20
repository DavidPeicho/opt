#pragma once

#include <array>

#include <albedo/wgpu.h>
#include <albedo/backend/object.h>

namespace albedo
{

namespace backend
{

template <uint8_t EntriesCount>
class BindGroupLayout: public WGPUObject<WGPUBindGroupId>
{

  public:

    BindGroupLayout() noexcept;

    ~BindGroupLayout() noexcept;

  public:

    void
    create(WGPUDeviceId deviceId);

    void
    setEntry(uint8_t at, WGPUShaderStage visiblity, WGPUBindingType type);

  private:

    std::array<WGPUBindGroupLayoutEntry, EntriesCount> m_entries;
    WGPUBindGroupLayoutDescriptor m_descriptor;

};

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
