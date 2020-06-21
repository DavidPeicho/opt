#pragma once

#include <array>
#include <functional>

#include <albedo/wgpu.h>
#include <albedo/backend/object.h>

namespace albedo
{

namespace backend
{

// TODO: create a `Device` class used as a factoryu to create every object.
// The device should first return a ObjectBuilder and then the builder should
// return the object? Sounds about right like that.
// It should then be possible to re-generate completly an object using the
// device class, or simply by forwarding an entire Descriptor?
// This would allow to avoid keeping the descriptors which can actually be
// pretty heavy.

template <uint8_t EntriesCount>
class BindGroupLayout: public WGPUObject<WGPUBindGroupLayoutDescriptor>
{

  public:

    BindGroupLayout() noexcept;

    ~BindGroupLayout() noexcept;

  public:

    void
    create(WGPUDeviceId deviceId);

    void
    setEntry(uint8_t at, WGPUShaderStage visiblity, WGPUBindingType type);

    void
    setEntries(const std::initializer_list<WGPUBindGroupLayoutEntry>&);

  private:

    std::array<WGPUBindGroupLayoutEntry, EntriesCount> m_entries;

};

// TODO: move to sources and use PIMPL.

template <uint8_t EntriesCount>
class BindGroup: public WGPUObject<WGPUBindGroupDescriptor>
{
  public:

    BindGroup() noexcept;

    ~BindGroup() noexcept;

  public:

    void
    create(WGPUDeviceId deviceId, WGPUBindGroupLayoutId);

    void
    setEntry(uint8_t at, WGPUBindingResource resource);

    void
    setEntries(const std::initializer_list<WGPUBindGroupEntry>&);

  private:

    std::array<WGPUBindGroupEntry, EntriesCount> m_entries;

};

} // namespace backend

} // namespace albedo

#include <albedo/backend/bind-group.hxx>
