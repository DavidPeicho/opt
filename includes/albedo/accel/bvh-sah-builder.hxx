#include <algorithm>
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
  float normalized = (centerOnAxis - centroidBoxMinOnAxis) / spanOnAxis;
  auto index = std::clamp(
    static_cast<Mesh::IndexType>(normalized * static_cast<float>(BinCount)),
    static_cast<Mesh::IndexType>(0),
    BinCount - 1
  );
  return index;
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
    aabb.expand(bin.aabb);
    nbTriangles += bin.nbPrimitives;
    bin.rightCost = static_cast<float>(nbTriangles) * aabb.getSurfaceArea();
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
    aabb.expand(bin.aabb);
    nbTriangles += bin.nbPrimitives;
    // SAH theory states that the cost is relative to the probability of
    // intersecting the sub area. However, we are simply comparing the cost,
    // so the division can be skipped.
    auto cost = ((float)nbTriangles * aabb.getSurfaceArea()) + bins[i + 1].rightCost;
    if (cost < minCost)
    {
      minCost = cost;
      splitIndex = i + 1;
    }
  }

  return splitIndex;
}

template <Mesh::IndexType BinCount>
Mesh::IndexType
recursiveBuild(
  std::vector<BVHNode>& nodes,
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

  Mesh::IndexType nodeIndex = nodes.size();
  nodes.emplace_back(BVHNode{});

  auto& node = nodes[nodeIndex];
  node.primitiveStartIndex = BVHNode::InternalNodeMask;
  node.aabb = box;
  node.center = box.center();

  // The split is based on the largest dimension.
  const auto splitAxis = centroidsBox.maximumExtent();
  const auto splitAxisLen = centroidsBox.max[splitAxis] - centroidsBox.min[splitAxis];

  // @todo: if the axis len is too small, split in half.

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
    const auto centerOnAxis = node.center[splitAxis];
    const auto binIndex = getBinIndex<BinCount>(
      centerOnAxis,
      centroidsBox.min[splitAxis],
      splitAxisLen
    );
    auto& bin = bins[binIndex];
    bin.nbPrimitives++;
    bin.aabb.expand(node.aabb);
  }

  auto splitIndex = findBestSplit<BinCount>(bins, start, end);
  // std::cout << "ITERATING" << std::endl;
  auto iterator = std::partition(
    nodes.begin() + start,
    nodes.begin() + end,
    [&](const BVHNode& node) {
      const auto centerOnAxis = node.center[splitAxis];
      const auto i = getBinIndex<BinCount>(centerOnAxis, centroidsBox.min[splitAxis], splitAxisLen);
      return i < splitIndex;
  });

  Mesh::IndexType mid = iterator - nodes.begin();
  if (mid <= start || mid >= end)
  {
    mid = (start + end) * 0.5;
  }

  const auto leftChild = recursiveBuild<BinCount>(nodes, bins, start, mid);
  const auto rightChild = recursiveBuild<BinCount>(nodes, bins, mid, end);
  node.leftChild = leftChild;
  node.rightChild = rightChild;

  float leftSurface = std::numeric_limits<float>::min();
  float rightSurface = std::numeric_limits<float>::min();
  if (leftChild != BVHNode::InternalNodeMask)
  {
    node.forestSize += 1 + nodes[leftChild].forestSize;
    leftSurface = nodes[leftChild].aabb.getSurfaceArea();
  }
  if (rightChild != BVHNode::InternalNodeMask)
  {
    node.forestSize += 1 + nodes[rightChild].forestSize;
    rightSurface = nodes[rightChild].aabb.getSurfaceArea();
  }

  if (rightSurface > leftSurface)
  {
    std::swap(node.leftChild, node.rightChild);
  }

  return nodeIndex;
}

}

template <Mesh::IndexType BinCount>
SAHBuilder<BinCount>::SAHBuilder(BVH& bvh)
  : m_bvh{bvh}
{ }

template <Mesh::IndexType BinCount>
void
SAHBuilder<BinCount>::build(const Mesh& mesh)
{
  auto nbTriangles = mesh.getTrianglesCount();
  if (nbTriangles == 0) { return; }

  const auto& vertices = mesh.getVertexBuffer();
  const auto& indices = mesh.getIndices();

  size_t nbNodes = 2 * nbTriangles - 1;
  std::vector<BVHNode> nodes;
  nodes.reserve(nbNodes);

  // TODO: create an iterator for the triangles.
  for (Mesh::IndexType i = 0; i < indices.size(); i += 3)
  {
    const auto& v0 = vertices[indices[i]].position;
    const auto& v1 = vertices[indices[i + 1]].position;
    const auto& v2 = vertices[indices[i + 2]].position;
    BVHNode node;
    node.primitiveStartIndex = i;
    node.aabb.expand(v0).expand(v1).expand(v2);
    node.center = node.aabb.center();
    node.leftChild = BVHNode::InternalNodeMask;
    node.rightChild = BVHNode::InternalNodeMask;
    nodes.emplace_back(std::move(node));
  }

  auto rootIndex = recursiveBuild<BinCount>(nodes, m_bins, 0, nodes.size());

  #if 0
  std::cout << "Root = " << rootIndex << std::endl;
  for (size_t i = 0; i < nodes.size(); ++i)
  {
    const auto& n = nodes[i];
    /* std::cout << "Node = " << i << std::endl;
    std::cout << " Primitive Index = " << n.primitiveIndex << std::endl;
    std::cout << " Span -> " << n.subtreeSize << std::endl;
    std::cout << " Left -> " << n.leftChild << std::endl;
    std::cout << " Right -> " << n.rightChild << std::endl; */
    std::cout << i << " -> " << n.leftChild << std::endl;
    std::cout << i << " -> " << n.rightChild << std::endl;
  }
  #endif

  m_bvh.nodes = std::move(nodes);
  m_bvh.rootIndex = rootIndex;
}

}

}
