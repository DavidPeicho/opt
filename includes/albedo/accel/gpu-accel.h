#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <albedo/wgpu.h>
#include <albedo/accel/bvh.h>
#include <albedo/mesh.h>
#include <albedo/scene.h>

namespace albedo
{

namespace accel
{

struct VertexGPU
{
  VertexGPU(const Vertex& vertex)
    : position{vertex.position}
  { }

  glm::vec3 position;
  uint32_t padding0;
};

struct BVHNodeGPU
{
  uint32_t primitiveIndex;
  uint32_t nextNodeIndex;
  glm::vec3 min;
  uint32_t padding_0;
  glm::vec3 max;
  uint32_t padding_1;
};

class GPUAccelerationStructure
{

  public:

    using NodesBuffer = std::vector<BVHNodeGPU>;

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
    std::vector<Vertex> m_vertices; // Vertices of **all** BVH.
    std::vector<Mesh::IndexBuffer> m_indices; // Indices of **all** BVH.
    NodesBuffer m_nodes; // Nodes of all BVH.

};

} // namespace accel

} // namespace albedo
