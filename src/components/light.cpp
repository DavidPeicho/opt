#include <albedo/components/light.h>

namespace albedo
{

namespace components
{

LightManager&
LightManager::createComponent(const Entity& entity)
{
  createComponent(entity, Light{});
  return *this;
}

LightManager&
LightManager::createComponent(const Entity& entity, Light&& data)
{
  auto instance = m_components.createComponent(entity, std::move(data));
  auto& component = m_components.components()[instance];
  return *this;
}

} // namespace backend

} // namespace albedo
