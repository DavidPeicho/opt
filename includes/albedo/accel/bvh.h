
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
  isLeaf() const { return primitiveStartIndex != InternalNodeMask; }

  // TODO: custom data for build should go in the builder.
  math::Box3 aabb;
  glm::vec3 center;

  size_t forestSize = 0;
  uint32_t primitiveStartIndex = 0;
  uint32_t leftChild = InvalidValue;
  uint32_t rightChild = InvalidValue;
};

struct BVH
{
  using NodeList = std::vector<BVHNode>;

  inline uint32_t
  depth(uint32_t start, uint32_t startDepth = 1) const
  {
    auto leftChildIndex = nodes[start].leftChild;
    auto rightChildIndex = nodes[start].rightChild;
    auto leftDepth = leftChildIndex != BVHNode::InvalidValue ? depth(leftChildIndex, startDepth + 1) : startDepth;
    auto rightDepth = rightChildIndex != BVHNode::InvalidValue ? depth(rightChildIndex, startDepth + 1) : startDepth;
    return std::max(leftDepth, rightDepth);
  }

  uint32_t rootIndex;
  std::vector<BVHNode> nodes;
};

}

}
