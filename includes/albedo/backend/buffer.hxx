namespace albedo
{

namespace backend
{

template <typename T>
Buffer<T>::Buffer() noexcept
{
  m_descriptor.label = "buffer";
  m_descriptor.size = 0;
  m_descriptor.usage = WGPUBufferUsage_STORAGE;

  m_readCallback = [&](WGPUBufferMapAsyncStatus status, uint8_t *data, uint8_t *userdata)
  {
    m_readPromise.set_value(status);
  };
  m_writeCallback = [&](WGPUBufferMapAsyncStatus status, uint8_t *data, uint8_t *userdata)
  {
    m_writePromise.set_value(status);
  };
}

template <typename T>
Buffer<T>::~Buffer() noexcept
{
  // TODO: destroy the object.
  wgpu_buffer_destroy(m_id);
}

template <typename T>
void
Buffer<T>::create(WGPUDeviceId deviceId)
{
  // TODO: destroy previous
  // TODO: add check for success.
  m_id = wgpu_device_create_buffer(deviceId, &m_descriptor);
}

template <typename T>
void
Buffer<T>::create(WGPUDeviceId deviceId, const std::vector<T>& data)
{
  // TODO: error checking **only** in debug mode.
  create(deviceId, &data[0], data.size());
}

template <typename T>
void
Buffer<T>::create(WGPUDeviceId deviceId, const T* const data, size_t count)
{
  // TODO: destroy previous
  // TODO: add check for success.
  auto size = sizeof (T) * count;
  m_descriptor.size = size;
  m_descriptor.mapped_at_creation = true;

  uint8_t* staging_memory = nullptr;
  m_id = wgpu_device_create_buffer(deviceId, &m_descriptor);

  // TODO: error checking.

  uint8_t *stagingMemory = wgpu_buffer_get_mapped_range(m_id, 0, size);
  auto* ptr = reinterpret_cast<const T* const>(data);
  std::memcpy(stagingMemory, ptr, size);
	wgpu_buffer_unmap(m_id);
}

template <typename T>
std::future<WGPUBufferMapAsyncStatus>
Buffer<T>::read(T* const dest, size_t count)
{
  return read(dest, 0, count);
}

template <typename T>
std::future<WGPUBufferMapAsyncStatus>
Buffer<T>::read(T* dest, size_t start, size_t count)
{
  // TODO: previous future is lost.
  // Warn user in debug mode?
  const size_t nbBytes = sizeof (T) * count;
  const auto startByte = sizeof (T) * start;
  assert(startByte + nbBytes <= getByteSize());

  wgpu_buffer_map_read_async(
    m_id, startByte, nbBytes, m_readCallback, reinterpret_cast<uint8_t*>(dest)
  );

  return m_readPromise.get_future();
}

template <typename T>
std::future<WGPUBufferMapAsyncStatus>
Buffer<T>::write(const T* const data, size_t count)
{
  return write(data, 0, count);
}

template <typename T>
std::future<WGPUBufferMapAsyncStatus>
Buffer<T>::write(const T* const data, size_t start, size_t count)
{
  // TODO: previous future is lost.
  // Warn user in debug mode?
  const size_t nbBytes = sizeof (T) * count;
  const auto startByte = sizeof (T) * start;
  assert(startByte + nbBytes <= getByteSize());

  wgpu_buffer_map_write_async(
    m_id, startByte, nbBytes, m_writeCallback, reinterpret_cast<uint8_t*>(data)
  );
  return m_writePromise.get_future();
}

template <typename T>
void
Buffer<T>::flush(WGPUQueueId queueId, const T* const data, size_t count)
{
  flush(queueId, data, 0, count);
}

template <typename T>
void
Buffer<T>::flush(WGPUQueueId queueId, const T* const data, size_t start, size_t count)
{
  const auto* datau8 = reinterpret_cast<const uint8_t*>(data);
  const size_t nbBytes = sizeof (T) * count;
  const auto startByte = sizeof (T) * start;
  wgpu_queue_write_buffer(queueId, m_id, startByte, datau8, nbBytes);
}

template <typename T>
WGPUBindGroupEntry
Buffer<T>::createBindGroupEntry(uint32_t binding, size_t offset) const
{
  return WGPUBindGroupEntry {
    .binding = binding,
    .buffer = m_id,
    .size = getByteSize(),
    .offset = offset,
    .sampler = 0,
    .texture_view = 0
  };
}

} // namespace backend

} // namespace albedo
