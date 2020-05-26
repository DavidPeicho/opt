#include <algorithm>

#include <albedo/scene.h>
#include <albedo/accel/bvh-sah-builder.h>

namespace albedo
{

namespace
{

using namespace accel;

// TODO: cleanup function. Starts to be difficult to track what starts where
// with all those offsets.
void
flattenBVH(
  std::vector<BVHNodeGPU>& output,
  const std::vector<BVHNode>& inputs,
  Mesh::IndexType nodeId,
  Mesh::IndexType lastSibling,
  Mesh::IndexType indicesOffset,
  Mesh::IndexType nodeOffset
)
{
  const BVHNode& node = inputs[nodeId];

  output.emplace_back(BVHNodeGPU {
    .primitiveStartIndex = BVHNode::InvalidValue,
    .max = node.aabb.max,
    .min = node.aabb.min,
    .nextNodeIndex = BVHNode::InvalidValue,
    .oldIndex = nodeId
  });

  auto& resultNode = output.back();

  if (node.leftChild != BVHNode::InvalidValue)
  {
    const auto& leftChild = inputs[node.leftChild];
    auto siblingIndex = leftChild.forestSize + (output.size() - nodeOffset) + 1;
    flattenBVH(output, inputs, node.leftChild, siblingIndex, indicesOffset, nodeOffset);
  }

  // resultNode.nextNodeIndex = nodeOffset + output.size();

  if (node.rightChild != BVHNode::InvalidValue)
  {
    flattenBVH(output, inputs, node.rightChild, lastSibling, indicesOffset, nodeOffset);
  }

  if (node.isLeaf())
  {
    resultNode.primitiveStartIndex = indicesOffset + node.primitiveStartIndex;
  }

  resultNode.nextNodeIndex = lastSibling;
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
      BVHNode::InvalidValue,
      startIndices,
      startNodes
    );
    m_nodes.back().nextNodeIndex = BVHNode::InvalidValue;

    startIndices += indices.size();
    startVertices += vertices.size();
    startNodes += bvh.nodes.size();
  }

  // #DEBUG
  for (size_t i = 0; i < m_nodes.size(); ++i)
  {
    const auto& node = m_nodes[i];
    std::cout << "Node - Old Index " << node.oldIndex << " - New Index " << i << std::endl;
    std::cout << " Primitive Index = " << node.primitiveStartIndex << std::endl;
    std::cout << " Min(" << glm::to_string(node.min) << ")" << std::endl;
    std::cout << " Max(" << glm::to_string(node.max) << ")" << std::endl;
    std::cout << " Next -> " << node.nextNodeIndex << std::endl;
  }
  // #ENDDEBUG
}

} // namespace albedo
