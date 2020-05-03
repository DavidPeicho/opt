#pragma once

#include "renderer.h"

namespace albedo
{

template <>
Renderer&
Renderer::deleteResource<GeometryInstance>(GeometryInstance resource)
{
  auto pos = std::find(
    this->m_Geometries.begin(),
    this->m_Geometries.end(),
    resource
  );
  if (pos != this->m_Geometries.end()) { this->m_Geometries.erase(pos); }
  return *this;
}

} // namespace albedo
