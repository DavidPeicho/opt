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

struct Transform : public Component<Transform>
{
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

    TransformManager&
    addChild(Entity child, Entity parent);

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
