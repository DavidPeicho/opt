
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
  math::AABB aabb;
  bool isLeaf;
};

class BVH
{

  private:

    std::vector<BVHNode> m_nodes;

};

}

}
