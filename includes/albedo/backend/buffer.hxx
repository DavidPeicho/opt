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
  m_descriptor.size = sizeof (T) * count;

  uint8_t* staging_memory = nullptr;
  m_id = wgpu_device_create_buffer_mapped(
    deviceId,
    &m_descriptor,
    &staging_memory
  );

  auto* ptr = reinterpret_cast<const T* const>(data);
  std::memcpy(staging_memory, ptr, m_descriptor.size);
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
  wgpu_queue_write_buffer(queueId, datau8, nbBytes, m_id, startByte);
}

} // namespace backend

} // namespace albedo
