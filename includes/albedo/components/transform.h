#pragma once

#include <optional>

#include <glm/glm.hpp>
#include <albedo/components/component-structure.h>
#include <albedo/mesh.h>

namespace albedo
{

namespace components
{

// TODO: convert to structure of array?
struct Transform
{
  glm::mat4 modelToLocal;
  glm::mat4 localToWorld;
  std::optional<Instance> parent;
  bool isDirty;
};

// TODO: make a base manager class. No need to make dynamic dispatch though,
// there is no logic in having manager with polymorphism.
class TransformManager
{

  public:

    TransformManager&
    createComponent(Instance instance);

    TransformManager&
    createComponent(Instance instance, Transform&&);

    void
    computeWorldTransforms();

    void
    computeWorldTransforms(Instance instance);

  public:

  private:

    ComponentArray<Transform> m_components;

};

} // namespace components

} // namespace albedo
