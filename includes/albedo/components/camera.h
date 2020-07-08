#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace albedo
{

namespace components
{

struct PerspectiveCamera
{
  PerspectiveCamera(float vFOV = 0.78) : vFOV(vFOV)
  { }

  float vFOV;
};

} // namespace components

} // namespace albedo
