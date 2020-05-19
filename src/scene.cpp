#include <albedo/scene.h>

namespace albedo
{

Scene::Scene(const ResourceManager::Ptr& manager)
     : m_resourceManager{manager}
{ }

Scene&
Scene::addInstance(Instance instance, InstanceData&& data)
{
  m_data.add(instance, std::move(data));
  m_transforms.add(instance, Transform{});
  return *this;
}

Scene&
Scene::deleteInstance(Instance instance)
{
  m_data.remove(instance);
  m_transforms.remove(instance);
  return *this;
}

// TODO: expose a renderable manager directly.
// TODO: improve API by letting user set the instanc with the mesh?
Scene&
Scene::addRenderable(Instance instance, size_t meshIndex)
{
  Renderable r;

  // TODO: cache the offset computation?
  uint32_t BVHRootOffset = 0;
  for (size_t i = 0; i < meshIndex; ++i)
  {
    BVHRootOffset += m_resourceManager->m_meshes[i]->getBVH().nodes.size();
  }

  r.bvhRootIndex = BVHRootOffset;
  r.materialIndex = 0;
  m_renderables.add(instance, std::move(r));

  return *this;
}

// TODO: expose a renderable manager directly.
Scene&
Scene::deleteRenderable(Instance instance)
{
  m_renderables.remove(instance);
  return *this;
}

} // namespace albedo
