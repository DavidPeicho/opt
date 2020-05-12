#pragma once

#include <albedo/wgpu.h>
#include <albedo/scene.h>

namespace albedo
{

namespace math
{

struct AABB
{
  glm::vec3 min;
  glm::vec3 max;
};

} // namespace math

} // namespace albedo
