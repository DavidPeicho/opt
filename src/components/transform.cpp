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
  auto& dataOpt = m_components.data(instance);
  if (!dataOpt) { return; }

  auto& data = *dataOpt;
  if (!data.isDirty) { return; }

  data.isDirty = false;

  // TODO: maybe the case where the parent data is none shouldnt happen,
  // i.e: the parent isnt added yet to the list.
  if (!data.parent || !m_components.data(data.parent))
  {
    data.localToWorld = data.modelToLocal;
    return;
  }

  computeWorldTransforms(data.parent);
  auto& parentData = *m_components.data(data.parent);
  data.localToWorld = parentData.localToWorld * data.modelToLocal;
}

} // namespace backend

} // namespace albedo
