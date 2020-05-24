#include <algorithm>

#include <albedo/scene.h>
#include <albedo/accel/bvh-sah-builder.h>

namespace albedo
{

namespace
{

using namespace accel;

void
flattenBVH(
  std::vector<BVHNodeGPU>::iterator& output,
  const std::vector<BVHNode>& inputs,
  Mesh::IndexType index,
  Mesh::IndexType nextIndex,
  Mesh::IndexType indexOffset,
  Mesh::IndexType nodeOffset
)
{
  const BVHNode& node = inputs[index];

  BVHNodeGPU result;
  result.max = node.aabb.max;
  result.min = node.aabb.min;
  result.nextNodeIndex = nodeOffset + nextIndex;

  if (node.isLeaf())
  {
    result.primitiveIndex = indexOffset + node.primitiveIndex;
    result.nextNodeIndex = BVHNode::InvalidValue;
    *output = std::move(result);
    output++;
    return;
  }

  if (node.leftChild != BVHNode::InvalidValue)
  {
    flattenBVH(output, inputs, node.leftChild, node.rightChild, indexOffset, nodeOffset);
  }
  if (node.rightChild != BVHNode::InvalidValue)
  {
    flattenBVH(output, inputs, node.rightChild, nextIndex, indexOffset, nodeOffset);
  }

  *output = std::move(result);
  output++;
}

}

Scene&
Scene::addInstance(Instance instance, InstanceData&& data)
{
  m_data.add(instance, std::move(data));
  m_transforms.add(instance, Transform{});
  return *this;
}

Scene&
Scene::deleteInstance(Instance instance)
{
  m_data.remove(instance);
  m_transforms.remove(instance);
  return *this;
}

// TODO: expose a renderable manager directly.
// TODO: improve API by letting user set the instanc with the mesh?
Scene&
Scene::addRenderable(Instance instance, size_t meshIndex)
{
  Renderable r;

  // TODO: cache the offset computation?
  uint32_t BVHRootOffset = 0;
  for (size_t i = 0; i < meshIndex; ++i)
  {
    BVHRootOffset += m_meshes[i]->getBVH().nodes.size();
  }

  r.bvhRootIndex = BVHRootOffset;
  r.materialIndex = 0;
  m_renderables.add(instance, std::move(r));

  return *this;
}

// TODO: expose a renderable manager directly.
Scene&
Scene::deleteRenderable(Instance instance)
{
  m_renderables.remove(instance);
  return *this;
}

Scene&
Scene::addMeshes(const std::vector<Mesh::MeshPtr>& meshes)
{
  // TODO: replace by a mesh to BVH tracking.
  for (const auto& m: meshes) { m_meshes.emplace_back(m); }
  return *this;
}

void
Scene::build()
{
  // TODO: should we store a ptr to the BVH inside each scene?
  // So scene can share BVH, and BVH contains the mesh.
  // Obviously, it would be up to the user to update the GPU acceleration when
  // needed.

  // TODO: move somewhere else. Shouldnt be created and destroyed.
  std::vector<BVH> bvhs(m_meshes.size());

  size_t totalNumberIndices = 0;
  size_t totalNumberVertices = 0;
  size_t totalNumberNodes = 0;
  for (const auto& m: m_meshes)
  {
    SAHBuilder<2> builder;
    auto bvh = builder.build(*m);
    totalNumberVertices += m->getVertexBuffer().size();
    totalNumberNodes += bvh.nodes.size();
    totalNumberIndices += m->getIndices().size();
    bvhs.emplace_back(std::move(bvh));
  }

  // TODO: need a way to map from mesh to the first index of its data in
  // the indices, vertices, and nodes array

  m_vertices.clear();
  m_vertices.reserve(totalNumberVertices);
  m_indices.clear();
  m_indices.reserve(totalNumberIndices);
  m_nodes.clear();
  m_nodes.reserve(totalNumberNodes);

  Mesh::IndexType startIndices = 0;
  Mesh::IndexType startVertices = 0;
  Mesh::IndexType startNodes = 0;
  // TODO: parralele for.
  for (size_t i = 0; i < bvhs.size(); ++i)
  {
    const auto& mesh = *m_meshes[i];
    const auto& bvh = mesh.getBVH();
    const auto& indices = mesh.getIndices();
    const auto& vertices = mesh.getVertexBuffer();

    std::copy(indices.begin(), indices.end(), m_indices.begin() + startIndices);
    std::copy(vertices.begin(), vertices.end(), m_vertices.begin() + startVertices);

    flattenBVH(m_nodes, bvhs[i].nodes, bvhs[i].rootIndex, startIndices, startNodes);

    startIndices += indices.size();
    startVertices += vertices.size();
    startNodes += bvh.nodes.size();
  }
}

} // namespace albedo
