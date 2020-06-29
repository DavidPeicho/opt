#include <albedo/backend/texture.h>

namespace albedo
{

namespace backend
{

TextureSampler::TextureSampler() noexcept
{
}

TextureSampler::~TextureSampler() noexcept
{
  // TODO
}

void
TextureSampler::create(WGPUDeviceId deviceId)
{
  m_id = wgpu_device_create_sampler(deviceId, &m_descriptor);
}

WGPUBindingResource
TextureSampler::getBindingResource() const
{
  return WGPUBindingResource {
    .tag = WGPUBindingResource_Sampler,
    .sampler = m_id
  };
}

TextureView::TextureView(WGPUTextureId textureId) noexcept
  : m_textureId{textureId}
{
  m_descriptor.format = WGPUTextureFormat_Rgba8UnormSrgb;
  m_descriptor.dimension = WGPUTextureViewDimension_D2;
  m_descriptor.aspect = WGPUTextureAspect_All;
  m_descriptor.base_mip_level = 0;
  m_descriptor.level_count = 0;
  m_descriptor.base_array_layer = 0;
  m_descriptor.array_layer_count = 0;
}

TextureView::~TextureView() noexcept
{
  // TODO: destroy view.
}

void
TextureView::create()
{
  // TODO: destroy previous?
  m_id = wgpu_texture_create_view(m_textureId, &m_descriptor);
}

WGPUBindingResource
TextureView::getBindingResource() const
{
  return WGPUBindingResource {
    .tag = WGPUBindingResource_TextureView,
    .texture_view = m_id
  };
}

Texture::Texture() noexcept
{
  m_descriptor.label = "";
  m_descriptor.size = WGPUExtent3d { .width = 0,
    .height = 0,
    .depth = 1
  };
  m_descriptor.mip_level_count = 1;
  // TODO: fill an issue. If this is left out, an overflow
  // occurs here: https://github.com/gfx-rs/wgpu/blob/master/wgpu-core/src/conv.rs#L438
  m_descriptor.sample_count = 1;
  m_descriptor.dimension = WGPUTextureDimension_D2;
  m_descriptor.format = WGPUTextureFormat_Rgba8UnormSrgb;
  m_descriptor.usage = WGPUTextureUsage_SAMPLED;
}

Texture::~Texture() noexcept
{
  // TODO: destroys all views.
  // TODO: destroy the object.
}

void
Texture::create(WGPUDeviceId deviceId)
{
  m_id = wgpu_device_create_texture(deviceId, &m_descriptor);
  for (auto& view: m_views)
  {
    view.m_textureId = m_id;
    view.create();
  }
}

TextureView&
Texture::createView() noexcept
{
  m_views.emplace_back(TextureView{m_id});
  return m_views.back();
}

TextureView&
Texture::createDefaultView() noexcept
{
  TextureView view(m_id);
  view.setFormat(m_descriptor.format);
  m_views.emplace_back(std::move(view));
  return m_views.back();
}

void
Texture::destroyView(TextureView& view) noexcept
{
  auto it = find(m_views.begin(), m_views.end(), view);
  if (it != m_views.end())
  {
    m_views.erase(it);
  }
}

void
Texture::destroyView(size_t index)
{
  m_views.erase(m_views.begin() + index);
}

} // namespace backend

} // namespace albedo
