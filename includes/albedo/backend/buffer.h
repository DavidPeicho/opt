#pragma once

#include <albedo/wgpu.h>
#include <future>

namespace albedo
{

namespace backend
{

// TODO: create a base class for all resources?
// TODO: move to sources and use PIMPL.
// TODO: use Builder pattern to enforce creating a buffer that is already
// sanely configured.
template <class T>
class Buffer
{
  public:

    Buffer() noexcept;

    ~Buffer() noexcept;

  public:

    void
    create(WGPUDeviceId deviceId);

    // TODO: use a trait to accept all object that can index contiguous
    // elements: (vector, array, etc...)
    void
    create(WGPUDeviceId deviceId, const std::vector<T>& data);

    // TODO: implement a `View` type to represent pointer + len?
    void
    create(WGPUDeviceId deviceId, const T* const data, size_t count);

    inline void
    setUsage(WGPUBufferUsage usage) { m_descriptor.usage = usage; }

    inline void
    setSize(size_t elementCount)
    {
      m_descriptor.size = sizeof (T) * elementCount;
    }

    inline size_t
    getByteSize() const { return m_descriptor.size; }

    inline size_t
    getElementCount() const { return m_descriptor.size / sizeof (T); }

    std::future<WGPUBufferMapAsyncStatus>
    read(T* const dest, size_t count);

    std::future<WGPUBufferMapAsyncStatus>
    read(T* const dest, size_t start, size_t count);

    std::future<WGPUBufferMapAsyncStatus>
    write(const T* const data, size_t count);

    std::future<WGPUBufferMapAsyncStatus>
    write(const T* const data, size_t start, size_t count);

    void
    flush(WGPUQueueId queueId, const T* const data, size_t count);

    void
    flush(WGPUQueueId queueId, const T* const data, size_t start, size_t count);

    WGPUBindingResource
    getBindingResource(size_t offset = 0) const;

    inline WGPUBindGroupId
    id() const { return m_id; }

  private:

    WGPUBufferId m_id;
    WGPUBufferDescriptor m_descriptor;

    std::function<void(WGPUBufferMapAsyncStatus, uint8_t*, uint8_t*)> m_readCallback;
    std::function<void(WGPUBufferMapAsyncStatus, uint8_t*, uint8_t*)> m_writeCallback;
    std::promise<WGPUBufferMapAsyncStatus> m_readPromise;
    std::promise<WGPUBufferMapAsyncStatus> m_writePromise;

};

} // namespace backend

} // namespace albedo

#include <albedo/backend/buffer.hxx>
