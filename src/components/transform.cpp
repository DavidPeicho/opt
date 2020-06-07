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
  auto instance = m_components.createComponent(entity, std::move(data));
  auto& component = m_components.components()[instance];
  component.isDirty = true;
  return *this;
}

TransformManager&
TransformManager::attach(const Entity& entity, const Entity& parent)
{
  auto parentInstance = m_components.getOrCreateComponent(parent);
  auto instance = m_components.getOrCreateComponent(entity);

  auto& data = m_components.components()[instance];
  // TODO: detach directly with instance instead of entity.
  if (data.parent) { detach(entity); }

  // TODO: when the component array changes, i.e: when a component is removed,
  // we **need** to update the parent reference...

  data.parent = parentInstance;
  data.isDirty = true;

  return *this;
}

TransformManager&
TransformManager::detach(const Entity& entity)
{
  // TODO: convert parent model to a sibling / child references.
  // When the new model is used, children should be detached automatically.
  auto data = m_components.getComponentData(entity);
  if (data)
  {
    (*data).parent = std::nullopt;
    (*data).isDirty = true;
  }
  return *this;
}

TransformManager&
TransformManager::detachChildren(const Entity& entity)
{
  auto instance = m_components.getComponent(entity);
  if (!instance) { return *this; }

  auto& components = m_components.components();
  for (TransformId i(0); i < components.size(); ++i)
  {
    auto& childData = components[i];
    if (childData.parent == instance)
    {
      // TODO: detach using entity directly instead of using another lookup.
      detach(m_components.entities()[i]);
    }
  }
  return *this;
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
