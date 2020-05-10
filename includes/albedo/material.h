#pragma once

#include <glm/glm.hpp>

namespace albedo
{

struct Material
{
  // TODO: create special struct for color to handle color spaces and
  // conversions.
  glm::vec3 color;
};

} // namespace albedo
