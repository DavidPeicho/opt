#include <albedo/components/transform.h>

namespace albedo
{

namespace components
{

TransformManager&
TransformManager::createComponent(const Entity& entity)
{
  createComponent(entity, Transform{});
  return *this;
}

TransformManager&
TransformManager::createComponent(const Entity& entity, Transform&& data)
{
  m_components.createComponent(entity, std::move(data));
  return *this;
}

TransformManager&
TransformManager::addChild(Entity child, Entity parent)
{
  reutrn *this;
}

void
TransformManager::computeWorldTransforms()
{
  auto& components = m_components.components();
  for (TransformId i(0); i < components.size(); ++i)
  {
    // TODO: use iterator here or something better.
    computeWorldTransforms(i);
  }
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
  const auto& components = m_components.components();
  const auto instance = m_components.getComponent(entity);
  if (!instance) { return std::nullopt; }
  return components[*instance].localToWorld;
}

void
TransformManager::computeWorldTransforms(const TransformId& instance)
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

  const auto& parentData = components[*data.parent];
  data.localToWorld = parentData.localToWorld * data.modelToLocal;
}

} // namespace backend

} // namespace albedo
