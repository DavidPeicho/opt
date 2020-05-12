#include "gpu-accel.h"

#include <albedo/accel/bvh-sah-builder.h>

namespace albedo
{

GPUAccelerationStructure::GPUAccelerationStructure(WGPUDeviceId deviceId)
                        : m_deviceID{deviceId}
{ }

void
GPUAccelerationStructure::build(const Scene& scene)
{
  const auto& meshes = scene.meshes().all();

  std::vector<BVH> BVHs;
  BVHs.reserve(meshes.size());

  for (size_t i = 0; i < BVHs.size(); ++i)
  {
    accel::SAHBuilder builder;
    BVH bvh = builder.build(*meshes[i]);
    BVHs[i] = std::move(bvh);
  }

}

void
BVH::build(const Mesh& mesh)
{
  
}

} // namespace albedo
