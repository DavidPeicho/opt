
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
  static constexpr Mesh::IndexType InternalNodeMask = 0xFFFFFFFF;
  static constexpr Mesh::IndexType InvalidValue = 0xFFFFFFFF;

  inline bool
  isLeaf() const { return primitiveIndex != InternalNodeMask; }

  // TODO: custom data for build should go in the builder.
  math::Box3 aabb;
  glm::vec3 center;

  Mesh::IndexType primitiveIndex = 0;
  Mesh::IndexType leftChild = 0;
  Mesh::IndexType rightChild = 0;
};

struct BVH
{
  Mesh::IndexType rootIndex;
  std::vector<BVHNode> nodes;
};

}

}
