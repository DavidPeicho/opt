#include <albedo/components/transform.h>

namespace albedo
{

namespace components
{

TransformManager&
TransformManager::createComponent(const Entity& entity)
{
  createComponent(instance, TransformData{ });
}

TransformManager&
TransformManager::createComponent(const Entity& entity, TransformData&& data)
{
  m_components.add(entity, std::move(data));
}

void
TransformManager::computeWorldTransforms()
{
  auto& components = m_components.components();
  for (auto& c: components) { computeWorldTransforms(c); }
}

void
TransformManager::computeWorldTransforms(const Entity& entity)
{
  const auto instance = m_components.getComponent(entity);
  if (instance)
  {
    computeWorldTransforms(*instance);
  }
}

OptionalRef<const glm::mat4>
TransformManager::getWorldMatrix(const Entity& entity) const
{
  const auto instance = m_components.getComponent(entity);
  return instance ? m_components.components()[instance] : std::nullopt;
}

void
TransformManager::computeWorldTransforms(TransformId instance)
{
  auto& components = m_components.components();
  auto& data = components[instance];
  if (!data.isDirty) { return; }

  data.isDirty = false;

  if (!data.parent)
  {
    data.localToWorld = data.modelToLocal;
    return;
  }

  computeWorldTransforms(*data.parent);

  const auto& parentData = components.data[*data.parent];
  data.localToWorld = parentData.localToWorld * data.modelToLocal;
}

} // namespace backend

} // namespace albedo
