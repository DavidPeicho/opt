#include <albedo/debug/scene-debugger.h>

namespace albedo
{

namespace debug
{

void
SceneDebugger::writeGraphiz(const accel::BVH& bvh, const std::string& filepath)
{
  std::stringstream ss;
  ss << "strict graph { " << std::endl;

  std::function<void(uint32_t)> appendNode = [&ss, &bvh, &appendNode](uint32_t index)
  {
    const auto& node = bvh.nodes[index];
    if (node.leftChild != accel::BVHNode::InvalidValue)
    {
      ss << index << " -- " << node.leftChild << std::endl;
      appendNode(node.leftChild);
    }
    if (node.rightChild != accel::BVHNode::InvalidValue)
    {
      ss << index << " -- " << node.rightChild << std::endl;
      appendNode(node.rightChild);
    }
  };
  appendNode(bvh.rootIndex);

  ss << std::endl << "}" << std::endl;

  std::ofstream f(filepath);
  f << ss.str();
}

void
SceneDebugger::writeGraphiz(const Scene& scene, uint32_t meshIndex, const std::string& filepath)
{
  std::stringstream ss;
  ss << "digraph BST { " << std::endl;

  // TODO: cache the starting point of each BVH root.
  uint32_t root = 0;
  for (size_t i = 0; i < meshIndex; ++i)
  {
    root += scene.m_meshes[i]->getBVH().nodes.size();
  }
  uint32_t end = root + scene.m_meshes[meshIndex]->getBVH().nodes.size();

  std::function<void(uint32_t)> process =
    [end, &ss, &scene, &process](uint32_t i)
  {
    if (i >= end)
    {
      throw "Error";
    }
    const auto& node = scene.m_nodes[i];
    if (node.nextNodeIndex != accel::BVHNode::InvalidValue)
    {
      ss << i << " -> " << node.nextNodeIndex << "[ color=\"red\" ]" << std::endl;
      if (node.primitiveStartIndex != accel::BVHNode::InvalidValue)
      {
        process(node.nextNodeIndex);
      }
    }
    if (node.primitiveStartIndex == accel::BVHNode::InvalidValue)
    {
      ss << i << " -> " << i + 1 << std::endl;
      process(i + 1);
    }
  };
  process(root);

  ss << std::endl << "}" << std::endl;
  std::ofstream f(filepath);
  f << ss.str();
}

} // namespace debug

} // namespace albedo
