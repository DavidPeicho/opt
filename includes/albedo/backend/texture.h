#include <vector>

#include <albedo/wgpu.h>
#include <albedo/backend/object.h>

namespace albedo
{

namespace backend
{

// Add basic inheritance just for `id`.
// TODO: move to sources and use PIMPL.
class TextureView: public WGPUObject<WGPUTextureViewDescriptor>
{
  public:

    friend class Texture;

  public:

    ~TextureView() noexcept;

  public:

    void
    create();

    inline TextureView&
    setDescriptor(const WGPUTextureViewDescriptor& descriptor)
    {
      m_descriptor = descriptor;
      return *this;
    }

    inline TextureView&
    setFormat(WGPUTextureFormat format)
    {
      m_descriptor.format = format;
      return *this;
    }

    inline TextureView&
    setDimension(WGPUTextureViewDimension dimension)
    {
      m_descriptor.dimension = dimension;
      return *this;
    }

    inline TextureView&
    setDimension(WGPUTextureAspect aspect)
    {
      m_descriptor.aspect = aspect;
      return *this;
    }

  private:

    TextureView(WGPUTextureId textureId) noexcept;

  private:

    WGPUTextureId m_textureId;

};

// TODO: move to sources and use PIMPL.
class Texture
{

  public:

    Texture() noexcept;

    ~Texture() noexcept;

  public:

    void
    create(WGPUDeviceId deviceId);

    TextureView&
    createView() noexcept;

    void
    destroyView(TextureView&) noexcept;

    void
    destroyView(size_t);

    inline WGPURenderPipelineId
    id() const { return m_id; }

  public:

    inline Texture&
    setDescriptor(const WGPUTextureDescriptor& descriptor)
    {
      m_descriptor = descriptor;
      return *this;
    }

    inline TextureView&
    getView(size_t index) { return m_views[index]; }

  private:

    WGPURenderPipelineId m_id;
    WGPUTextureDescriptor m_descriptor;

    std::vector<TextureView> m_views;
};

} // namespace backend

} // namespace albedo
