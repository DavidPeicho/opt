#include <albedo/debug/scene-debugger.h>

namespace albedo
{

namespace debug
{

void
writeGraphiz(const accel::BVH& bvh, const std::string& filepath)
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

} // namespace debug

} // namespace albedo
