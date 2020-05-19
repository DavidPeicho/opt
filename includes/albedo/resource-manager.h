#pragma once

#include <memory>
#include <vector>

#include <albedo/mesh.h>

namespace albedo
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

class ResourceManager
{
  friend class Scene;

  public:

    using NodesBuffer = std::vector<BVHNodeGPU>;
    using Ptr = std::shared_ptr<ResourceManager>;

  public:

    void
    addMeshes(const std::vector<Mesh::MeshPtr>& meshes);

    // TODO: user should provide the BVH himself, so that he can choose
    // what object are static or not.
    void
    build();

  private:

    std::vector<Mesh::MeshPtr> m_meshes;

    std::vector<Vertex> m_vertices; // Vertices of **all** BVH.
    std::vector<Mesh::IndexBuffer> m_indices; // Indices of **all** BVH.
    NodesBuffer m_nodes; // Nodes of all BVH.

};

}
