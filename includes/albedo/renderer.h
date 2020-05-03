#pragma once

#include <vector>

#include "geometry.h"

namespace albedo
{

class Renderer
{

  public:

    GeometryInstance
    addGeometry(Geometry&& geometry);

    template <typename T>
    Renderer&
    deleteResource(T resource);

  private:

    std::vector<Geometry> m_Geometries;

};

} // namespace filament
