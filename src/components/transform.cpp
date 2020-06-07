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
TransformManager::attach(const Entity& child, const Entity& parent)
{
  auto parentInstance = m_components.getComponent(parent);
  if (!parent) { parentInstance =  }
  return *this;
}

TransformManager&
TransformManager::detach(const Entity& entity)
{
  // TODO: convert parent model to a sibling / child references.
  // When the new model is used, children should be detached automatically.
}

TransformManager&
TransformManager::detachChildren(const Entity& entity)
{
  // TODO: convert parent model to a sibling / child references.
  // When the new model is used, children should be detached automatically.
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
