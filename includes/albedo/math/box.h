#pragma once

// TODO: add a separate class for printing.
#include <iostream>
#include <limits>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>

namespace albedo
{

namespace math
{

struct Box3
{

  Box3()
    : min{std::numeric_limits<float>::max()}
    , max{- std::numeric_limits<float>::min()}
  { }

  inline Box3&
  makeEmpty()
  {
    min = glm::vec3(std::numeric_limits<float>::max());
    max = glm::vec3(- std::numeric_limits<float>::min());
    return *this;
  }

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

  inline glm::vec3
  diagonal() { return max - min; }

  inline float
  getSurfaceArea()
  {
    const auto d = diagonal();
    return 2.0 * glm::length2(d);
  }

  inline uint8_t
  maximumExtent()
  {
    if (max.x > max.y && max.x > max.z) { return 0; }
    return max.y > max.z ? 1 : 2;
  }

  inline bool
  isEmpty()
  {
    return max.x <= min.x || max.y <= min.y || max.z <= min.z;
  }

  inline std::string
  toString()
  {
    return "min(" + glm::to_string(min) + "), max(" + glm::to_string(max) + ")";
  }

  glm::vec3 min;
  glm::vec3 max;

};

} // namespace math

} // namespace albedo
