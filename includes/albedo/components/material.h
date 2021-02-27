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
    : color{glm::vec4(1.0, 1.0, 1.0, 1.0)}
    , emission{glm::vec4(0.0, 0.0, 0.0, 0.0)}
    , metalnessFactor{1.0}
    , roughnessFactor{1.0}
    , albedoIndex{0xFFFFFFFF}
    , normalIndex{0xFFFFFFFF}
    , emissionIndex{0xFFFFFFFF}
    , metalRoughnessIndex{0xFFFFFFFF}
  { }

  // TODO: replace by custom class handling color space.
  glm::vec4 color;
  glm::vec4 emission;

  float metalnessFactor;
  float roughnessFactor;
  uint32_t albedoIndex;
  uint32_t normalIndex;
  uint32_t emissionIndex;
  uint32_t metalRoughnessIndex;
  uint32_t pad_0;
  uint32_t pad_1;
};

} // namespace components

} // namespace albedo
