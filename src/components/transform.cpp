#include <albedo/components/transform.h>

namespace albedo
{

namespace components
{

TransformManager&
TransformManager::createComponent(Instance instance)
{
  createComponent(instance, Transform{ });
}

TransformManager&
TransformManager::createComponent(Instance instance, Transform&& transform)
{
  m_components.add(instance, std::move(transform));
}

void
TransformManager::computeWorldTransforms()
{
  auto& entities = m_components.instances();
  for (auto& entity: entities) { computeWorldTransforms(entity); }
}

void
TransformManager::computeWorldTransforms(Instance instance)
{
  auto& data = m_components.data(instance);
  if (!data.isDirty) { return; }

  data.isDirty = false;

  if (!data.parent)
  {
    data.localToWorld = data.modelToLocal;
    return;
  }

  computeWorldTransforms(data.parent.value());
  auto& parentData = m_components.data(data.parent.value());
  data.localToWorld = parentData.localToWorld * data.modelToLocal;
}

} // namespace backend

} // namespace albedo
