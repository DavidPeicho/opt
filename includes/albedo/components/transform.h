#pragma once

#include <optional>

#include <glm/glm.hpp>
#include <albedo/components/component.h>
#include <albedo/components/component-structure.h>
#include <albedo/mesh.h>
#include <albedo/entity.h>
#include <albedo/utils/index.h>
#include <albedo/utils/optional-ref.h>

namespace albedo
{

namespace components
{

// TODO: convert parent model to a sibling / child references.
struct Transform : public Component<Transform>
{
  Transform()
  {
    modelToLocal = glm::mat4(1.0);
    localToWorld = glm::mat4(1.0);
    isDirty = false;
  }

  glm::mat4 modelToLocal;
  glm::mat4 localToWorld;
  std::optional<Transform::Id> parent;
  bool isDirty;
};

// TODO: make a base manager class. No need to make dynamic dispatch though,
// there is no logic in having manager with polymorphism.
class TransformManager
{

  public:

    using TransformId = typename Transform::Id;

  public:

    TransformManager&
    createComponent(const Entity&);

    TransformManager&
    createComponent(const Entity&, Transform&&);

    // TODO: implement `removeComponent`.

    TransformManager&
    attachTo(Entity child, Entity parent);

    TransformManager&
    detach(const Entity&);

    TransformManager&
    detachChildren(const Entity&);

    void
    computeWorldTransforms();

    void
    computeWorldTransforms(const Entity&);

  public:

    OptionalRef<const glm::mat4>
    getWorldMatrix(const Entity&) const;

  private:

    void
    computeWorldTransforms(const TransformId& instance);

  private:

    ComponentArray<Transform> m_components;

};

} // namespace components

} // namespace albedo
