
#pragma once

#include <vector>
#include <albedo/math/box.h>
#include <glm/glm.hpp>

namespace albedo
{

namespace accel
{

struct BVHNode
{
  static constexpr uint32_t InternalNodeMask = 0xFFFFFFFF;
  static constexpr uint32_t InvalidValue = 0xFFFFFFFF;

  inline bool
  isLeaf() const { return primitiveIndex != InternalNodeMask; }

  // TODO: custom data for build should go in the builder.
  math::Box3 aabb;
  glm::vec3 center;

  uint32_t primitiveIndex = 0;
  uint32_t leftChild = 0;
  uint32_t rightChild = 0;
};

struct BVH
{
  uint32_t rootIndex;
  std::vector<BVHNode> nodes;
};

}

}
