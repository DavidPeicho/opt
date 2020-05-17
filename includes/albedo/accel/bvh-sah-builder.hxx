#include <numeric>
#include <iostream>
namespace albedo
{

namespace accel
{

namespace
{

template <Mesh::IndexType BinCount>
inline Mesh::IndexType
getBinIndex(
  float centerOnAxis,
  float centroidBoxMinOnAxis,
  float spanOnAxis
)
{
  // TODO: what should we do when the span is 0...?
  if (spanOnAxis <= 0.000000001) { return BinCount / 2; }
  float normalized = (centerOnAxis - centroidBoxMinOnAxis) / spanOnAxis;
  return std::min(
    BinCount * static_cast<Mesh::IndexType>(normalized),
    BinCount - 1
  );
}

template <Mesh::IndexType BinCount>
Mesh::IndexType
findBestSplit(
  std::array<Bin, BinCount>& bins,
  Mesh::IndexType start,
  Mesh::IndexType end
)
{
  math::Box3 aabb;
  Mesh::IndexType nbTriangles = 0;

  //
  // Step 1: save the cost of splitting starting from the right side.
  // the cost is directly stored into the bin. By doing so, we can save one
  // stack allocation.
  //

  for (Mesh::IndexType i = BinCount - 1; i >= 1; --i)
  {
    auto& bin = bins[i];
    nbTriangles += bin.nbPrimitives;
    aabb.expand(bin.aabb);
    bin.rightCost = nbTriangles * aabb.getSurfaceArea();
  }

  //
  // Step 2: compute the cost of splitting from the left side.
  //
  // Compute the overall left + right side cost at each bin, and save the
  // lowest overall cost.
  //

  nbTriangles = 0;
  aabb.makeEmpty();

  Mesh::IndexType splitIndex = 0;
  auto minCost = std::numeric_limits<float>::max();

  for (Mesh::IndexType i = 0; i < BinCount - 1; ++i)
  {
    const auto& bin = bins[i];
    nbTriangles += bin.nbPrimitives;
    aabb.expand(bin.aabb);
    // SAH theory states that the cost is relative to the probability of
    // intersecting the sub area. However, we are simply comparing the cost,
    // so the division can be skipped.
    auto cost = (nbTriangles * aabb.getSurfaceArea()) + bin.rightCost;
    if (cost < minCost)
    {
      minCost = cost;
      splitIndex = i;
    }
  }

  return splitIndex;
}

template <Mesh::IndexType BinCount>
Mesh::IndexType
recursiveBuild(
  std::vector<BVHConstructionNode>& nodes,
  std::array<Bin, BinCount>& bins,
  Mesh::IndexType start,
  Mesh::IndexType end
)
{
  if (end - start <= 1) { return start; }

  math::Box3 box;
  math::Box3 centroidsBox;
  for (Mesh::IndexType i = start; i < end; ++i)
  {
    const auto& node = nodes[i];
    box.expand(node.aabb);
    centroidsBox.expand(node.center);
  }

  const auto axis = box.maximumExtent();

  // std::cout << centroidsBox.max[axis] << " " << centroidsBox.min[axis] << std::endl;

  const auto spanOnAxis = centroidsBox.max[axis] - centroidsBox.min[axis];

  //
  // Step 1: initializes every bin computing, for each triangle, its associated
  // bin. Each bin bounding box and number of primitives is updated.
  //

  for (auto& bin: bins)
  {
    bin.nbPrimitives = 0;
    bin.aabb.makeEmpty();
  }

  for (Mesh::IndexType i = start; i < end; ++i)
  {
    const auto& node = nodes[i];
    const auto centerOnAxis = node.center[axis];
    const auto binIndex = getBinIndex<BinCount>(
      centerOnAxis,
      centroidsBox.min[axis],
      spanOnAxis
    );
    auto& bin = bins[binIndex];
    bin.nbPrimitives++;
    bin.aabb.expand(node.aabb);
  }

  auto splitIndex = findBestSplit<BinCount>(bins, start, end);
  const BVHConstructionNode* middleNode = std::partition(
    nodes.begin() + start,
    nodes.begin() + end - 1,
    [&](const BVHConstructionNode& node) {
      const auto centerOnAxis = node.center[axis];
      const auto i = getBinIndex<BinCount>(centerOnAxis, centroidsBox.min[axis], spanOnAxis);
      return i <= splitIndex;
  });

  Mesh::IndexType mid = middleNode - &nodes[0];

  Mesh::IndexType nodeIndex = nodes.size();
  nodes.emplace_back(BVHConstructionNode{});

  auto& node = nodes[nodeIndex];
  node.primitiveIndex = BVHConstructionNode::InternalNodeMask;
  node.aabb = box;
  node.center = box.center();

  auto leftChild = recursiveBuild<BinCount>(nodes, bins, start, mid);
  auto rightChild = recursiveBuild<BinCount>(nodes, bins, mid, end);

  node.leftChild = leftChild;
  node.rightChild = rightChild;

  return nodeIndex;
}

}

template <Mesh::IndexType BinCount>
BVH
SAHBuilder<BinCount>::build(const Mesh& mesh)
{
  auto nbTriangles = mesh.getTrianglesCount();
  if (nbTriangles == 0) { return BVH{}; }

  // Initializes bin.
  for (auto& bin: m_bins)
  {
    bin.nbPrimitives = 0;
    bin.rightCost = 0.0;
  }

  const auto& vertices = mesh.getVertexBuffer();
  const auto& indices = mesh.getIndices();

  size_t nbNodes = 2 * nbTriangles - 1;
  std::vector<BVHConstructionNode> nodes;
  nodes.reserve(nbNodes);

  // TODO: create an iterator for the triangles.
  for (Mesh::IndexType i = 0; i < indices.size(); i += 3)
  {
    const auto& v0 = vertices[indices[i]].position;
    const auto& v1 = vertices[indices[i + 1]].position;
    const auto& v2 = vertices[indices[i + 2]].position;
    BVHConstructionNode node;
    node.primitiveIndex = i / 3;
    node.aabb.expand(v0).expand(v1).expand(v2);
    node.center = node.aabb.center();
    nodes.emplace_back(std::move(node));
  }

  auto rootIndex = recursiveBuild<BinCount>(nodes, m_bins, 0, nodes.size());

  std::cout << "Root = " << rootIndex << std::endl;

   for (size_t i = 0; i < nodes.size(); ++i) {
    const auto& n = nodes[i];
    std::cout << "Node " << i << " | Primitive = " << n.primitiveIndex << std::endl;
    std::cout << "  Left  -> " << n.leftChild << std::endl;
    std::cout << "  Right  -> " << n.rightChild << std::endl;
  }

  BVH bvh;
  return bvh;
}

}

}
