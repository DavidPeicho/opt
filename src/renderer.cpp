#include <albedo/renderer.h>

namespace albedo
{

Renderer::Renderer(
  WGPUDeviceId deviceId,
  WGPUSurfaceId surfaceId,
  uint32_t width,
  uint32_t height
)
  : m_deviceId{deviceId}
  , m_surfaceId{surfaceId}
{
  m_swapChainDescriptor.width = width;
  m_swapChainDescriptor.height = height;
}

Renderer&
Renderer::init()
{
  m_renderPipeline.init(m_deviceId);

  m_swapChainDescriptor.usage = WGPUTextureUsage_OUTPUT_ATTACHMENT;
  m_swapChainDescriptor.format = WGPUTextureFormat_Bgra8Unorm;
  m_swapChainDescriptor.present_mode = WGPUPresentMode_Fifo,

  // TODO: check for errors?
  m_swapChainId = wgpu_device_create_swap_chain(
    m_deviceId, m_surfaceId, &m_swapChainDescriptor
  );
}

Renderer&
Renderer::resize(uint32_t width, uint32_t height)
{
  const auto previousWidth = m_swapChainDescriptor.width;
  const auto previousHeight = m_swapChainDescriptor.height;
  if (width != previousWidth || height != previousHeight)
  {
    m_swapChainDescriptor.width = width;
    m_swapChainDescriptor.height = height;
    m_swapChainId = wgpu_device_create_swap_chain(
      m_deviceId, m_surfaceId, &m_swapChainDescriptor
    );
  }
  return *this;
}

Renderer&
Renderer::startFrame()
{
  WGPUSwapChainOutput nextSwapTexture = wgpu_swap_chain_get_next_texture(m_swapChainId);
  // TODO: add error handling
  if (!nextSwapTexture.view_id) { return *this; }
}

Renderer&
Renderer::endFrame()
{
  wgpu_swap_chain_present(m_swapChainId);
}


} // namespace albedo
