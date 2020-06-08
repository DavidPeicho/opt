#pragma once

#include <optional>

#include <glm/glm.hpp>
#include <albedo/components/component.h>

namespace albedo
{

namespace components
{

// TODO: convert parent model to a sibling / child references.
struct Material : public Component<Material>
{
  Material()
    : color{glm::vec3(1.0, 1.0, 1.0)}
  { }

  // TODO: replace by custom class handling color space.
  glm::vec3 color;
};

} // namespace components

} // namespace albedo
