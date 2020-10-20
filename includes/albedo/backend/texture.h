#pragma once

#include <vector>

#include <albedo/wgpu.h>
#include <albedo/backend/object.h>

namespace albedo
{

namespace backend
{

class TextureSampler: public WGPUObject<WGPUSamplerDescriptor>
{
  public:

    TextureSampler() noexcept;
    ~TextureSampler() noexcept;

  public:

    void
    create(WGPUDeviceId);

  public:

    WGPUBindGroupEntry
    createBindGroupEntry(uint32_t binding) const;

};

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

    WGPUBindGroupEntry
    createBindGroupEntry(uint32_t binding) const;

  private:

    TextureView(WGPUTextureId textureId) noexcept;

  private:

    WGPUTextureId m_textureId;

};

// TODO: move to sources and use PIMPL.
class Texture: public WGPUObject<WGPUTextureDescriptor>
{

  public:

    Texture() noexcept;

    ~Texture() noexcept;

  public:

    void
    create(WGPUDeviceId deviceId);

    TextureView&
    createView() noexcept;

    TextureView&
    createDefaultView() noexcept;

    void
    destroyView(TextureView&) noexcept;

    void
    destroyView(size_t);

  public:

    inline void
    setWidth(uint32_t width) { m_descriptor.size.width = width; }

    inline void
    setHeight(uint32_t height) { m_descriptor.size.height = height; }

    inline void
    setDepth(uint32_t depth) { m_descriptor.size.height = depth; }

    inline void
    setDimensions(uint32_t width, uint32_t height, uint32_t depth)
    {
      setWidth(width);
      setHeight(height);
      setDepth(depth);
    }

    inline TextureView&
    getView(size_t index) { return m_views[index]; }

  private:

    std::vector<TextureView> m_views;

};

} // namespace backend

} // namespace albedo
