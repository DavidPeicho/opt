#include <albedo/scene.h>

namespace albedo
{

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
  m_meshes.remove(instance);
  return *this;
}

Scene&
Scene::addMesh(Instance instance, Mesh&& mesh)
{
  auto ptr = std::make_shared<Mesh>(std::move(mesh));
  return addMesh(instance, ptr);
}

Scene&
Scene::addMesh(Instance instance, Mesh::MeshPtr& meshPtr)
{
  auto ptr = meshPtr;
  m_meshes.add(instance, std::move(ptr));
  return *this;
}

Scene&
Scene::removeMesh(Instance instance)
{
  m_meshes.remove(instance);
  return *this;
}

} // nanespace albedo
