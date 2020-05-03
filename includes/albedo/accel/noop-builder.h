#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace albedo
{

namespace accel
{

class NoopBuilder
{

  public:

    void build(
      const& std::vector<glm::vec3> vertices,
      const& std::vector<size_t> indices
    );

};

}

}
