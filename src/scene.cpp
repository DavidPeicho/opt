#include <algorithm>

#include <glm/gtx/string_cast.hpp>

#include <albedo/scene.h>
#include <albedo/accel/bvh-sah-builder.h>

namespace albedo
{

namespace
{

using namespace accel;

class FlattenTask {

  public:

    inline
    FlattenTask(
      EntryOffsetTable<BVHNodeGPU>& output,
      Mesh::IndexType primitiveIndexStart
    )
      : m_output{output}
      , m_start(output.data.size())
      , m_curr(m_start)
      , m_primitiveIndexStart(primitiveIndexStart)
    {
      // TODO: not super clean... We don't really know who manages what with
      // this EntryOffsetClass. To refactor.
      m_output.entries.push_back(m_curr);
    }

  public:

    void
    flatten(const BVH::NodeList& inputs, Mesh::IndexType rootNode)
    {
      flatten(inputs, rootNode, BVHNode::InvalidValue);
    }

    void
    flatten(
      const BVH::NodeList& inputs,
      Mesh::IndexType inputIndex,
      Mesh::IndexType missIndex
    )
    {
      const BVHNode& node = inputs[inputIndex];

      m_output.data.insert(m_output.data.begin() + m_curr, BVHNodeGPU {
        .min = node.aabb.min,
        .max = node.aabb.max,
        .nextNodeIndex = BVHNode::InvalidValue,
        .primitiveStartIndex = BVHNode::InvalidValue
        // .oldIndex = inputIndex
      });
      auto& resultNode = m_output.data[m_curr];
      m_curr++;

      if (node.leftChild != BVHNode::InvalidValue)
      {
        const auto& leftChild = inputs[node.leftChild];
        auto childMissIndex = leftChild.forestSize + count() + 1;
        flatten(inputs, node.leftChild, childMissIndex);
      }
      if (node.rightChild != BVHNode::InvalidValue)
      {
        flatten(inputs, node.rightChild, missIndex);
      }
      if (node.isLeaf())
      {
        resultNode.primitiveStartIndex = getRelativePrimitiveIndex(node.primitiveStartIndex);
      }
      resultNode.nextNodeIndex = missIndex != BVHNode::InvalidValue ?
        getRelativeIndex(missIndex)
        : BVHNode::InvalidValue;
    }

  private:

    inline Mesh::IndexType
    getRelativeIndex(Mesh::IndexType index)
    {
      return m_start + index;
    }

    inline Mesh::IndexType
    getRelativePrimitiveIndex(Mesh::IndexType index)
    {
      return m_primitiveIndexStart + index;
    }

    inline Mesh::IndexType
    count() { return m_curr - m_start; }

  private:

    EntryOffsetTable<BVHNodeGPU>& m_output;
    Mesh::IndexType m_start;
    Mesh::IndexType m_curr;
    Mesh::IndexType m_primitiveIndexStart;

};

}

Scene&
Scene::addInstance(const Entity& entity, InstanceData&& data)
{
  m_data.createComponent(entity, std::move(data));
  return *this;
}

Scene&
Scene::deleteInstance(const Entity& entity)
{
  m_data.removeComponent(entity);
  return *this;
}

// TODO: expose a renderable manager directly.
// TODO: improve API by letting user set the instanc with the mesh?
// TODO: not great API here, because the BVH **has** to be built before calling this method.
Scene&
Scene::addRenderable(const Entity& entity, size_t meshIndex)
{
  Renderable r;
  r.materialIndex = 0;
  r.meshIndex = meshIndex;
  m_renderables.createComponent(entity, std::move(r));
  return *this;
}

// TODO: expose a renderable manager directly.
Scene&
Scene::deleteRenderable(const Entity& entity)
{
  m_renderables.removeComponent(entity);
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

// TODO: store begin / end of each BVH in the flattened structure?
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
  m_vertices.data.reserve(totalNumberVertices);
  m_indices.clear();
  m_indices.data.reserve(totalNumberIndices);
  m_nodes.clear();
  m_nodes.data.reserve(totalNumberNodes);

  // TODO: parralele for.
  for (size_t i = 0; i < m_meshes.size(); ++i)
  {
    auto& mesh = *m_meshes[i];
    const auto& bvh = mesh.getBVH();
    const auto& indices = mesh.getIndices();
    const auto& vertices = mesh.getVertexBuffer();

    m_indices.push(indices);
    m_vertices.push(vertices);

    auto flattenTask = FlattenTask(
      m_nodes,
      m_vertices.entries[i]
    );
    flattenTask.flatten(bvh.nodes, bvh.rootIndex);
  }

  #if 0
  for (size_t i = 0; i < m_indices.size(); i += 3)
  {
    const auto& v0 = m_vertices[m_indices[i]].position;
    const auto& v1 = m_vertices[m_indices[i + 1]].position;
    const auto& v2 = m_vertices[m_indices[i + 2]].position;
    std::cout << glm::to_string(v0) << glm::to_string(v1) << glm::to_string(v2) << std::endl;
  }
  #endif

  #if 0
  for (const auto& node: m_nodes) {
    if (node.primitiveStartIndex != 0xFFFFFFFF) {
      std::cout << node.primitiveStartIndex << std::endl;
    }
  }
  #endif

  #if 0
  for (size_t i = 0; i < m_nodes.size(); ++i)
  {
    const auto& node = m_nodes[i];
    // std::cout << "Node - Old Index " << node.oldIndex << " - New Index " << i << std::endl;
    std::cout << " Primitive Index = " << node.primitiveStartIndex << std::endl;
    std::cout << " Min(" << glm::to_string(node.min) << ")" << std::endl;
    std::cout << " Max(" << glm::to_string(node.max) << ")" << std::endl;
    std::cout << " Next -> " << node.nextNodeIndex << std::endl;
  }
  #endif
}

Scene&
Scene::update()
{
  // TODO: update only when dirty.
  m_transformManager.computeWorldTransforms();

  const auto& renderableComponents = m_renderables.components();
  const auto& renderableEntities = m_renderables.entities();
  m_instances.resize(renderableEntities.capacity());
  for (size_t i = 0; i < renderableComponents.size(); ++i)
  {
    const auto& data = renderableComponents[i];
    const auto entity = renderableEntities[i];
    auto& gpuInstance = m_instances[i];

    // TODO: adding the bvhRoot on each update shouldn't be needed.
    gpuInstance.bvhRootIndex = m_nodes.entries[data.meshIndex];
    gpuInstance.materialIndex = data.materialIndex;

    const auto transformData = m_transformManager.getWorldMatrix(entity);
    if (transformData)
    {
      gpuInstance.modelToWorld = *transformData;
    }
  }

  for (const auto& i: m_instances)
  {
    std::cout << "instances" << std::endl;
    std::cout << glm::to_string(i.modelToWorld) << std::endl;
  }

  return *this;
}

} // namespace albedo
