#include <albedo/backend/bind-group.h>

namespace albedo
{

namespace backend
{

template <uint8_t NbEntries>
BindGroup<NbEntries>::BindGroup() noexcept
{
  m_bindGroupDescriptor.label = "bind group";
  m_bindGroupDescriptor.layout = bindLayoutGroupId,
  m_bindGroupDescriptor.entries = &m_entries[]
  m_bindGroupDescriptor.entries_length = 0;
}

template <uint8_t NbEntries>
BindGroup<NbEntries>::~RenderPipeline()
{
  // TODO: destroy the object.
  // Needs fix from wgpu-native.
}

template <uint8_t NbEntries>
void
BindGroup<NbEntries>::create(WGPUDeviceId deviceId, WGPUBindGroupLayoutId bindGroupLayoutId)
{
  m_bindGroupDescriptor.entries_length = m_entries.size();
  if (m_bindGroupDescriptor.entries_length > 0)
  {
    m_bindGroupDescriptor.entries = &m_entries[0];
  }
  m_bindGroupDescriptor.layout = bindGroupLayoutId;

  // TODO: add check for success.
  m_id = wgpu_device_create_bind_group(deviceId, &m_bindGroupDescriptor)
}

template <uint8_t NbEntries, typename T>
void
BindGroup<NbEntries>::setEntry<T>(T&& entry, uint8_t at)
{
  m_entries[at] = std::foward<T>(entry);
}

} // namespace backend

} // namespace albedo
