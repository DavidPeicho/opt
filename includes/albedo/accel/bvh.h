
#pragma once

#include <vector>
#include <albedo/math/box.h>
#include <glm/glm.hpp>

namespace albedo
{

namespace accel
{

class BVHNode
{

};

struct GPUBVHNode
{
  glm::vec3 min;
  uint32_t primitiveIndex;
  glm::vec4 max;
  uint32_t nextNodeIndex;
};

class BVH
{

  private:

    std::vector<BVHNode> m_nodes;

};

}

}
