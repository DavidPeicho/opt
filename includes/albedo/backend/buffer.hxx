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
Buffer<T>::create(WGPUDeviceId deviceId, const T* const data, size_t count)
{
  // TODO: destroy previous
  // TODO: add check for success.
  m_descriptor.size = sizeof (T) * count;

  T* staging_memory = nullptr;
  WGPUBufferId buffer = wgpu_device_create_buffer_mapped(
    deviceId,
    &m_descriptor,
    &staging_memory
  );
  std::memcpy(staging_memory, data, m_descriptor.size);
	wgpu_buffer_unmap(buffer);

  return buffer;
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

} // namespace backend

} // namespace albedo
