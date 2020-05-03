#include "renderer.h"

namespace albedo
{

GeometryInstance
Renderer::addGeometry(Geometry&& geometry)
{
  this->m_Geometries.push_back(std::move(geometry));
  return geometry.getInstance();
}

} // namespace albedo
