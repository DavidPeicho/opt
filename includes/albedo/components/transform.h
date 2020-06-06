#pragma once

#include <optional>

#include <glm/glm.hpp>
#include <albedo/components/component-structure.h>
#include <albedo/mesh.h>
#include <albedo/entity.h>
#include <albedo/utils/index.h>
#include <albedo/utils/optional-ref.h>

namespace albedo
{

namespace components
{

// TODO: Should we rename `Component` into `ComponentInstance` or
// `ComponentIndex` for clarity?

// TODO: move ID class into a ComponentData::using.
class TransformId : public Index
{
  public:

    TransformId(Index::Type value) noexcept : Index(value) { }
};

// TODO: convert to structure of array?
struct TransformData
{
  glm::mat4 modelToLocal;
  glm::mat4 localToWorld;
  std::optional<TransformId> parent;
  bool isDirty;
};

// TODO: make a base manager class. No need to make dynamic dispatch though,
// there is no logic in having manager with polymorphism.
class TransformManager
{

  public:

    TransformManager&
    createComponent(const Entity&);

    TransformManager&
    createComponent(const Entity&, TransformData&&);

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

    ComponentArray<TransformId, TransformData> m_components;

};

} // namespace components

} // namespace albedo
