#include <albedo/wgpu.h>
#include <future>

namespace albedo
{

namespace backend
{

// TODO: move to sources and use PIMPL.

template <uint8_t NbEntries>
class BindGroup
{
  public:

    BindGroup() noexcept;

    ~BindGroup() noexcept;

  public:

    template <class T>
    void
    setEntry(T&& entry, uint8_t at);

    void
    create(WGPUDeviceId deviceId, WGPUBindGroupLayoutId bindGroupLayoutId);

  private:

    WGPUBindGroupId m_id;

    std::array<WGPUBindGroupEntry, NbEntries> m_entries;

};

} // namespace backend

} // namespace albedo

#include <albedo/backend/bind-group.hxx>
