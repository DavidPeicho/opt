#pragma once

#include <limits>

#include <glm/glm.hpp>

#include <albedo/wgpu.h>
#include <albedo/scene.h>

namespace albedo
{

namespace math
{

struct Box3
{

  inline Box3&
  expand(glm::vec3 point)
  {
    min = glm::min(min, point);
    max = glm::max(max, point);
    return *this;
  }

  inline Box3&
  expand(const Box3& box)
  {
    expand(box.min);
    expand(box.max);
    return *this;
  }

  inline glm::vec3
  center()
  {
    return (min + max) * 0.5f;
  }

  glm::vec3 min;
  glm::vec3 max;

};

} // namespace math

} // namespace albedo
