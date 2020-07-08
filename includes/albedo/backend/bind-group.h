#pragma once

#include <initializer_list>

#include <albedo/wgpu.h>
#include <albedo/backend/object.h>

namespace albedo
{

namespace backend
{

// TODO: create a `Device` class used as a factory to create every object.
// The device should first return a ObjectBuilder and then the builder should
// return the object? Sounds about right like that.
// It should then be possible to re-generate completly an object using the
// device class, or simply by forwarding an entire Descriptor?
// This would allow to avoid keeping the descriptors which can actually be
// pretty heavy.

class BindGroupLayout: public WGPUObject<WGPUBindGroupLayoutDescriptor>
{

  public:

    using Ptr = std::shared_ptr<BindGroupLayout>;
    using Entries = std::initializer_list<WGPUBindGroupLayoutEntry>;

  public:

    BindGroupLayout() noexcept
    {
      m_descriptor.label = "bind group layout";
      m_descriptor.entries = nullptr;
      m_descriptor.entries_length = 0;
    }

    ~BindGroupLayout() noexcept
    {
      // TODO: destroy the object.
    }

  public:

    inline void
    create(WGPUDeviceId deviceId, const Entries& entries)
    {
      m_descriptor.entries_length = entries.size();
      m_descriptor.entries = entries.size() > 0 ? entries.begin() : nullptr;
      // TODO: add check for success.
      m_id = wgpu_device_create_bind_group_layout(deviceId, &m_descriptor);
    }

};

// TODO: move to sources and use PIMPL.

class BindGroup: public WGPUObject<WGPUBindGroupDescriptor>
{
  public:

    using Entries = std::initializer_list<WGPUBindGroupEntry>;
    using BindGroupLayoutPtr = std::shared_ptr<BindGroupLayout>;

  public:

    BindGroup() noexcept
    {
      m_descriptor.label = "bind group";
      m_descriptor.layout = 0;
      m_descriptor.entries = nullptr;
      m_descriptor.entries_length = 0;
    }

    ~BindGroup() noexcept
    {
      // TODO: destroy the object.
      // Needs fix from wgpu-native.
    }

  public:

    inline void
    create(WGPUDeviceId deviceId, const Entries& entries)
    {
      m_descriptor.entries_length = entries.size();
      m_descriptor.entries = entries.size() > 0 ? entries.begin() : nullptr;
      m_descriptor.layout = m_layout != nullptr ? m_layout->id() : 0;
      // TODO: add check for success.
      m_id = wgpu_device_create_bind_group(deviceId, &m_descriptor);
    }

    inline void
    setLayout(const std::shared_ptr<BindGroupLayout>& layout)
    {
      m_layout = layout;
    }

    inline const std::shared_ptr<BindGroupLayout>&
    getLayout() const { return m_layout; }

  private:

    BindGroupLayoutPtr m_layout;

};

} // namespace backend

} // namespace albedo
