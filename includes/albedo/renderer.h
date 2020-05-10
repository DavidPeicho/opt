#pragma once

#include <vector>

namespace albedo
{

class Renderer
{

  public:

    template <typename T>
    Renderer&
    deleteResource(T resource);

};

} // namespace filament
