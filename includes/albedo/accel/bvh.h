
#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace albedo
{

namespace accel
{

struct AABB
{
  glm::vec3 min;
  glm::vec3 max;
};

class BVHNode
{
  
};

class BVH
{

  private:

    std::vector<BVHNode> m_nodes;

};

}

}
