#include <utility>

namespace albedo
{

namespace backend
{

template <uint8_t EntriesCount>
BindGroupLayout<EntriesCount>::BindGroupLayout() noexcept
{
  m_descriptor.label = "bind group layout";
  m_descriptor.entries = nullptr;
  m_descriptor.entries_length = EntriesCount;
}

template <uint8_t EntriesCount>
BindGroupLayout<EntriesCount>::~BindGroupLayout() noexcept
{
  // TODO: destroy the object.
}

template <uint8_t EntriesCount>
void
BindGroupLayout<EntriesCount>::create(WGPUDeviceId deviceId)
{
  m_descriptor.entries_length = m_entries.size();
  if (m_descriptor.entries_length > 0)
  {
    m_descriptor.entries = &m_entries[0];
  }
  // TODO: add check for success.
  m_id = wgpu_device_create_bind_group_layout(deviceId, &m_descriptor);
}

template <uint8_t EntriesCount>
void
BindGroupLayout<EntriesCount>::setEntry(
  uint8_t at,
  WGPUShaderStage visibility,
  WGPUBindingType type
)
{
  m_entries[at] = WGPUBindGroupLayoutEntry {
    .binding = at,
    .visibility = visibility,
    .ty = type
  };
}

template <uint8_t EntriesCount>
void
BindGroupLayout<EntriesCount>::setEntries(
  const std::initializer_list<WGPUBindGroupLayoutEntry>& entries
)
{
  for (const auto& entry: entries) { m_entries[entry.binding] = entry; }
}

template <uint8_t EntriesCount>
BindGroup<EntriesCount>::BindGroup() noexcept
{
  m_descriptor.label = "bind group";
  m_descriptor.layout = 0;
  m_descriptor.entries = nullptr;
  m_descriptor.entries_length = 0;
}

template <uint8_t NbEntries>
BindGroup<NbEntries>::~BindGroup() noexcept
{
  // TODO: destroy the object.
  // Needs fix from wgpu-native.
}

template <uint8_t EntriesCount>
void
BindGroup<EntriesCount>::create(WGPUDeviceId deviceId, WGPUBindGroupLayoutId layoutId)
{
  m_descriptor.entries_length = m_entries.size();
  if (m_descriptor.entries_length > 0)
  {
    m_descriptor.entries = &m_entries[0];
  }
  m_descriptor.layout = layoutId;

  // TODO: add check for success.
  m_id = wgpu_device_create_bind_group(deviceId, &m_descriptor);
}

template <uint8_t EntriesCount>
void
BindGroup<EntriesCount>::setEntry(uint8_t at, WGPUBindingResource resource)
{
  m_entries[at] = WGPUBindGroupEntry { .binding = at, .resource = resource };
}

template <uint8_t EntriesCount>
void
BindGroup<EntriesCount>::setEntries(
  const std::initializer_list<WGPUBindGroupEntry>& entries
)
{
  for (const auto& entry: entries) { m_entries[entry.binding] = entry; }
}

} // namespace backend

} // namespace albedo
