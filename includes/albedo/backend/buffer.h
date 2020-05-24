#include <albedo/wgpu.h>
#include <future>

namespace albedo
{

namespace backend
{

// TODO: move to sources and use PIMPL.
template <class T>
class Buffer
{
  public:

    Buffer() noexcept;

    ~Buffer() noexcept;

  public:

    void
    create(WGPUDeviceId deviceId);

    inline void
    setUsage(WGPUBufferUsage usage) { m_descriptor.usage = usage; }

    inline size_t
    getByteSize() { return m_descriptor.size; }

    inline size_t
    getElementCount() { return m_descriptor.size / sizeof (T); }

    std::future<WGPUBufferMapAsyncStatus>
    read(T* const dest, size_t count);

    std::future<WGPUBufferMapAsyncStatus>
    read(T* const dest, size_t start, size_t count);

    std::future<WGPUBufferMapAsyncStatus>
    write(const T* const data, size_t count);

    std::future<WGPUBufferMapAsyncStatus>
    write(const T* const data, size_t start, size_t count);

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
