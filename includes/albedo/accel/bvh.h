
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

class BVH
{

  public:

    inline size_t
    getNodesCount() { return m_nodes.size(); }

  private:

    std::vector<BVHNode> m_nodes;

};

}

}
