#pragma once

#include "geometry.h"

namespace albedo
{

namespace scene
{

Geometry::Geometry(
  std::vector<glm::vec3> vertices,
  std::vector<glm::vec3> normals,
  std::vector<size_t> indices
)
  : _vertices{std::move(vertices)}
  , _normals{std::move(normals)}
  , _indices{std::move(indices)}
{}

} // nanespace scene

} // nanespace albedo
