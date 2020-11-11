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
      std::vector<BVHNodeGPU>& output,
      Mesh::IndexType primitiveIndexStart
    )
      : m_output{output}
      , m_start(output.size())
      , m_curr(m_start)
      , m_primitiveIndexStart(primitiveIndexStart)
    { }

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

      m_output.insert(m_output.begin() + m_curr, BVHNodeGPU {
        .min = node.aabb.min,
        .max = node.aabb.max,
        .nextNodeIndex = getRelativeIndex(missIndex),
        .primitiveStartIndex = BVHNode::InvalidValue
        // .oldIndex = inputIndex
      });
      auto& resultNode = m_output[m_curr];
      m_curr++;

      if (node.isLeaf())
      {
        resultNode.primitiveStartIndex = getRelativePrimitiveIndex(node.primitiveStartIndex);
        return;
      }

      if (node.leftChild != BVHNode::InvalidValue)
      {
        const auto& leftChild = inputs[node.leftChild];
        if (node.rightChild != BVHNode::InvalidValue)
        {
          auto childMissIndex = leftChild.forestSize + count() + 1;
          flatten(inputs, node.leftChild, childMissIndex);
        }
        else
        {
          flatten(inputs, node.leftChild, missIndex);
        }
      }
      if (node.rightChild != BVHNode::InvalidValue)
      {
        flatten(inputs, node.rightChild, missIndex);
      }
    }

  private:

    inline Mesh::IndexType
    getRelativeIndex(Mesh::IndexType index)
    {
      return index != BVHNode::InvalidValue ? m_start + index : BVHNode::InvalidValue;
    }

    inline Mesh::IndexType
    getRelativePrimitiveIndex(Mesh::IndexType index)
    {
      return m_primitiveIndexStart + index;
    }

    inline Mesh::IndexType
    count() { return m_curr - m_start; }

  private:

    std::vector<BVHNodeGPU>& m_output;
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
Scene::addRenderable(const Entity& entity, size_t meshIndex, size_t materialId)
{
  Renderable r;
  r.materialIndex = materialId;
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

Scene&
Scene::addMaterial(Material&& material)
{
  m_materials.emplace_back(std::move(material));
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

  #if 1
  std::cout << "[ Scene Info ]" << std::endl;
  #endif

  size_t totalNumberIndices = 0;
  size_t totalNumberVertices = 0;
  size_t totalNumberNodes = 0;
  for (auto& m: m_meshes)
  {
    // TODO: discard empty BVH.
    SAHBuilder builder(m->getBVH());
    builder.build(*m);

    totalNumberVertices += m->getVertexBuffer().size();
    totalNumberNodes += m->getBVH().nodes.size();
    totalNumberIndices += m->getIndices().size();

    #if 1
    const auto& bvh = m->getBVH();
    std::cout << "  -> BVH" << std::endl;
    std::cout << "     Nodes count = " << bvh.nodes.size() << std::endl;
    std::cout << "     Depth = " << bvh.depth(bvh.rootIndex) << std::endl;
    #endif
  }

  // TODO: clean all of that... it's a mess and not easy to understand.
  // I would prefer to loose a tiny bit of performance here and get some clean
  // code easy to understand.

  m_nodes.clear();
  m_nodes.reserve(totalNumberNodes);
  m_vertices.clear();
  m_vertices.reserve(totalNumberVertices);
  m_indices.clear();
  m_indices.resize(totalNumberIndices);

  size_t currIndex = 0;
  // TODO: parralele for.
  for (size_t i = 0; i < m_meshes.size(); ++i)
  {
    const auto& mesh = *m_meshes[i];
    const auto& indices = mesh.getIndices();
    const auto& vertices = mesh.getVertexBuffer();
    const auto startIndex = m_vertices.size();
    for (const auto primitive: indices)
    {
      m_indices[currIndex++] = primitive + startIndex;
    }
    m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());
  }

  // This is needed as well. BVH are already built pointing with nodes
  // pointing to other. THose node need to have the index updated.
  size_t startIndex = 0;
  // TODO: parralele for.
  for (size_t i = 0; i < m_meshes.size(); ++i)
  {
    auto& mesh = *m_meshes[i];
    const auto& bvh = mesh.getBVH();
    auto flattenTask = FlattenTask(
      m_nodes,
      startIndex
    );
    flattenTask.flatten(bvh.nodes, bvh.rootIndex);

    startIndex += mesh.getVertexBuffer().size();
  }
}

Scene&
Scene::update()
{
  // TODO: update only when dirty.
  m_transforms.computeWorldTransforms();

  // Update lights.
  const auto& lightComponents = m_lightsManager.m_components.components();
  const auto& lightEntities = m_lightsManager.m_components.entities();
  m_lights.resize(lightEntities.size());
  std::cout << "SENDING LIGHT" << std::endl;
  for (size_t i = 0; i < lightEntities.size(); ++i)
  {
    auto entity = lightEntities[i];
    const auto& data = lightComponents[i];
    auto& instance = m_lights[i];

    instance.intensity = data.intensity;
    instance.normal = glm::vec4(0.0, 0.0, 1.0, 0.0);
    instance.tangent = glm::vec4(1.0, 0.0, 0.0, 0.0);
    instance.bitangent = glm::vec4(0.0, - 1.0, 0.0, 0.0);

    auto transformData = m_transforms.getComponent(entity);
    if (transformData)
    {
      auto matrix = transformData->localToWorld;
      glm::vec4 origin = glm::vec4(transformData->getWorldPosition(), 1.0);
      instance.normal = matrix * glm::vec4(0.0, 0.0, 1.0, 0.0);
      instance.tangent = matrix * glm::vec4(data.width, 0.0, 0.0, 0.0);
      instance.bitangent = matrix * glm::vec4(0.0, - data.height, 0.0, 0.0);

      origin = origin - 0.5f * instance.tangent - 0.5f * instance.bitangent;

      // Pack origin into the normal, tangent, and bitangent vectors.
      instance.normal.w = origin.x;
      instance.tangent.w = origin.y;
      instance.bitangent.w = origin.z;
    }
  }

  const auto& renderableComponents = m_renderables.components();
  const auto& renderableEntities = m_renderables.entities();
  m_instances.resize(renderableEntities.size());
  for (size_t i = 0; i < renderableComponents.size(); ++i)
  {
    const auto& data = renderableComponents[i];
    const auto entity = renderableEntities[i];
    auto& gpuInstance = m_instances[i];

    gpuInstance.materialIndex = data.materialIndex;

    // TODO: cache the starting point of each BVH root.
    uint32_t bvhRootIndex = 0;
    for (size_t i = 0; i < data.meshIndex; ++i)
    {
      bvhRootIndex += m_meshes[i]->getBVH().nodes.size();
    }
    gpuInstance.bvhRootIndex = bvhRootIndex;

    // const auto& d = m_data.getComponentData(entity);
    // if (d) {
    //   std::cout << "Name = " << d->name << std::endl;
    // }
    // std::cout << "Root index = " << gpuInstance.bvhRootIndex << std::endl;

    const auto transformData = m_transforms.getWorldMatrix(entity);
    if (transformData)
    {
      // TODO: save inverse.
      gpuInstance.worldToModel = glm::inverse(*transformData);
      // std::cout << glm::to_string(gpuInstance.worldToModel) << std::endl;
    }
  }

  #if 1
  std::cout << "  -> Instances: " <<  m_instances.size() << std::endl;
  #endif

  return *this;
}

} // namespace albedo
