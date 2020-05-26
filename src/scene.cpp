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
  std::vector<BVHNodeGPU>& output,
  const std::vector<BVHNode>& inputs,
  Mesh::IndexType index,
  Mesh::IndexType nextIndex,
  Mesh::IndexType indexOffset,
  Mesh::IndexType nodeOffset,
  uint32_t oldIndex
)
{
  const BVHNode& node = inputs[index];

  BVHNodeGPU result;
  result.primitiveIndex = BVHNode::InvalidValue;
  result.max = node.aabb.max;
  result.min = node.aabb.min;
  result.nextNodeIndex = nextIndex != BVHNode::InvalidValue ?
      nodeOffset + nextIndex
      : BVHNode::InvalidValue;

  std::cout << "Node | old index = " << oldIndex << " | new index " << output.size() << std::endl;
  std::cout << " Primitive Index = " << node.primitiveIndex << std::endl;
  std::cout << " Min(" << glm::to_string(node.aabb.min) << ")" << std::endl;
  std::cout << " Max(" << glm::to_string(node.aabb.max) << ")" << std::endl;
  std::cout << " next -> " << nextIndex << std::endl;
  if (node.isLeaf()) {
    std::cout << " <leaf node>" << std::endl;
  }

  if (node.isLeaf())
  {
    result.primitiveIndex = indexOffset + node.primitiveIndex;
    output.emplace_back(std::move(result));
    return;
  }

  // std::cout << " Next -> " << node. << std::endl;

  output.emplace_back(std::move(result));
  if (node.leftChild != BVHNode::InvalidValue)
  {
    const auto& left = inputs[node.leftChild];
    flattenBVH(output, inputs, node.leftChild, left.subtreeSize + 1 + output.size(), indexOffset, nodeOffset, node.leftChild);
  }
  if (node.rightChild != BVHNode::InvalidValue)
  {
    flattenBVH(output, inputs, node.rightChild, nextIndex, indexOffset, nodeOffset, node.rightChild);
  }
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
Scene::addMesh(const Mesh::MeshPtr& mesh)
{
  // TODO: replace by a mesh to BVH tracking.
  m_meshes.emplace_back(mesh);
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

  size_t totalNumberIndices = 0;
  size_t totalNumberVertices = 0;
  size_t totalNumberNodes = 0;
  for (auto& m: m_meshes)
  {
    // TODO: discard empty BVH.
    SAHBuilder<2> builder(m->getBVH());
    builder.build(*m);
    totalNumberVertices += m->getVertexBuffer().size();
    totalNumberNodes += m->getBVH().nodes.size();
    totalNumberIndices += m->getIndices().size();
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
  for (size_t i = 0; i < m_meshes.size(); ++i)
  {
    auto& mesh = *m_meshes[i];
    const auto& bvh = mesh.getBVH();
    const auto& indices = mesh.getIndices();
    const auto& vertices = mesh.getVertexBuffer();

    m_indices.insert(m_indices.begin() + startIndices, indices.begin(), indices.end());
    m_vertices.insert(m_vertices.begin() + startVertices, vertices.begin(), vertices.end());

    std::vector<BVHNodeGPU>::iterator start = m_nodes.begin();
    flattenBVH(
      m_nodes,
      bvh.nodes,
      bvh.rootIndex,
      1,
      startIndices,
      startNodes,
      0
    );

    startIndices += indices.size();
    startVertices += vertices.size();
    startNodes += bvh.nodes.size();
  }

  // #DEBUG
  /* for (size_t i = 0; i < m_nodes.size(); ++i)
  {
    const auto& node = m_nodes[i];
    std::cout << "Node = " << i << std::endl;
    std::cout << " Primitive Index = " << node.primitiveIndex << std::endl;
    std::cout << " Min(" << glm::to_string(node.min) << ")" << std::endl;
    std::cout << " Max(" << glm::to_string(node.max) << ")" << std::endl;
    std::cout << " Next -> " << node.nextNodeIndex << std::endl;
  } */
  // #ENDDEBUG
}

} // namespace albedo
