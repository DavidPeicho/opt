#include <numerics>

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
  return std::max(BinCount * normalized, BinCount - 1);
}

template <Mesh::IndexType BinCount>
Mesh::IndexType
findBestSplit(
  const std::vector<BVHConstructionNode>& nodes,
  std::array<Bin, BinCount>& bins,
  Mesh::IndexType start,
  Mesh::IndexType end
)
{
  Box aabb;
  Mesh::IndexType nbTriangles = 0;

  //
  // Step 1: save the cost of splitting starting from the right side.
  // the cost is directly stored into the bin. By doing so, we can save one
  // stack allocation.
  //

  for (Mesh::IndexType i = BinCount - 1; i >= 1; --i)
  {
    const auto& bin = bins[i];
    nbTriangles += bin.nbPrimitives;
    aabb.expand(bin.aabb);
    bin.rightCost = nbTriangles * aabb.surfaceArea();
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
  auto minCost = std::numeric_limit<float>::max();

  for (Mesh::IndexType i = 0; i < BinCount - 1; ++i)
  {
    nbTriangles += bin.nbPrimitives;
    aabb.expand(bin.aabb);
    // SAH theory states that the cost is relative to the probability of
    // intersecting the sub area. However, we are simply comparing the cost,
    // so the division can be skipped.
    auto cost = (nbTriangles * rightAABB.surfaceArea()) + bin.righCost;
    if (cost < minCost)
    {
      minCost = cost;
      splitIndex = i;
    }
  }

  return splitIndex;
}

Mesh::IndexType
recBuild(
  std::vector<BVHConstructionNode>& nodes,
  Mesh::IndexType start,
  Mesh::IndexType end
)
{
  static constexpr Mesh::IndexType NB_BINS = 12;

  if (start == end)
  {
    // Leaf.
    return start;
  }

  math::Box3 box;
  math::Box3 centroidsBox;
  for (Mesh::IndexType i = start; i < end; ++i)
  {
    const auto& node = nodes[i];
    box.expand(node.aabb);
    centroidsBox.expand(node.center);
  }

  const auto axis = box.maximumExtent();
  const auto spanOnAxis = centroidsBox.max[axis] - centroidsBox.min[axis];

  //
  // Step 1: initializes every bin computing, for each triangle, its associated
  // bin. Each bin bounding box and number of primitives is updated.
  //

  for (const auto& node: nodes) {
    const auto centerOnAxis = node.center[axis];
    const auto binIndex = getBinIndex(
      centerOnAxis,
      centroidsBox.min[axis],
      spanOnAxis
    );
    auto& bin = m_bins[binIndex];
    bin.nbPrimitives++;
    bin.aabb.expand(node.aabb);
  }

  auto splitIndex = findBestSplit(nodes, m_bins, start, end);
  size_t rightIndex = std::partition(nodes + item.begin, primitive_indices + item.end, [&] (size_t i) {
    return compute_bin_index(centers[i], best_axis) < split_index;
}) - primitive_indices;

}

}

template <Mesh::IndexType BinCount>
BVH
SAHBuilder<BinCount>::build(const Mesh& mesh)
{
  auto nbTriangles = mesh.getTrianglesCount();
  if (nbTriangles == 0) { return BVH{}; }

  const auto& vertices = mesh.getVertexBuffer();
  const auto& indices = mesh.getIndices();

  size_t nbNodes = 2 * nbTriangles - 1;
  std::vector<BVHConstructionNode> nodes(nbNodes);
  // TODO: create an iterator for the triangles.
  for (Mesh::IndexType i = 0; i < indices.size(); i += 3)
  {
    const auto& v0 = vertices[i].position;
    const auto& v1 = vertices[i + 1].position;
    const auto& v2 = vertices[i + 2].position;
    BVHConstructionNode node;
    node.primitiveIndex = i;
    node.aabb.expand(v0).expand(v1).expand(v2);
    node.center = node.aabb.center();
    nodes.emplace_back(std::move(node));
    boundingBox.expand(node.aabb);
    centroidBoundingBox.expand(node.center);
  }

  BVH bvh;
  return bvh;
}

}

}
