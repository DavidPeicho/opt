#pragma once

#include <albedo/wgpu.h>
#include <albedo/scene.h>

namespace albedo
{

struct AABB
{
  glm::vec3 min;
  glm::vec3 max;
};

struct BVHNode
{
  AABB aabb;
	bool isLeaf;
};

struct BVHLeaf
{

};

// TODO: add compile-time selection of BVH building algorithm.
class BVH
{

  public:

    void
    build(const Mesh& mesh);

  private:

    std::vector<BVHNode> m_nodes;
};

class GPUAccelerationStructure
{

  public:

    GPUAccelerationStructure(WGPUDeviceId deviceId);

  public:

    // TODO: user should provide the BVH himself, so that he can choose
    // what object are static or not.
    void
    build(const Scene& scene);

    void
    updateInstances();

  private:
    WGPUDeviceId m_deviceID;

    std::vector<BVH> m_BVHs;

};

} // namespace albedo
