#pragma once

#include <glm/glm.hpp>

namespace albedo
{

struct Material
{
  // TODO: create special struct for color to handle color spaces and
  // conversions.
  glm::vec4 color;
};

} // namespace albedo
