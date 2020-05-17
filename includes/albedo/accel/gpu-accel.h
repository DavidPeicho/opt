#pragma once

#include <vector>

#include <albedo/wgpu.h>
#include <albedo/accel/bvh.h>
#include <albedo/mesh.h>
#include <albedo/scene.h>

namespace albedo
{

namespace accel
{

struct BVHNodeGPU
{
  glm::vec3 min;
  uint32_t primitiveIndex;
  glm::vec4 max;
  uint32_t nextNodeIndex;
};

class GPUAccelerationStructure
{

  public:

    // GPUAccelerationStructure(WGPUDeviceId deviceId);

  public:

    void
    addMeshes(const std::vector<Mesh::MeshPtr>& meshes);

    // TODO: user should provide the BVH himself, so that he can choose
    // what object are static or not.
    void
    build(const Scene& scene);

    void
    updateInstances();

  private:
    WGPUDeviceId m_deviceID;

    std::vector<Mesh::MeshPtr> m_meshes;
    std::vector<BVHNodeGPU> m_nodes;

};

} // namespace accel

} // namespace albedo
