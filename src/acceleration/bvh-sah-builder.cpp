#include <albedo/accel/bvh-sah-builder.h>

namespace albedo
{

namespace accel
{

namespace
{

struct BVHConstructionNode
{
  math::Box3 aabb;
  glm::vec3 center;

  Geometry::IndexType primitiveStartIndex = 0;
  Geometry::IndexType leftChild = 0;
  Geometry::IndexType rightChild = 0;
};

void
recBuild(
  std::vector<BVHConstructionNode>& nodes,
  Geometry::IndexType start,
  Geometry::IndexType end
)
{
  if (start == end)
  {
    // LEAF.
  }

  math::Box3 box;

  // TODO: parallelize that is asked by user.
  for (Geometry::IndexType i = start; i <= end; ++i)
  {
    box.expand(nodes[i].aabb);
  }
}

}

BVH
SAHBuilder::build(const Mesh& mesh)
{
  Geometry::IndexType nbTriangles = mesh.getTrianglesCount();
  if (nbTriangles == 0) { return BVH{}; }

  size_t nbNodes = 2 * nbTriangles - 1;

  // TODO: parallelize if ask by user.
  std::vector<BVHConstructionNode> nodes(nbNodes);
  for (const auto& geom: mesh.getGeometries())
  {
    const auto& vertices = mesh.getVertexBuffer();
    // TODO: create an iterator for the triangles.
    for (Geometry::IndexType i = 0; i < geom.indices.size(); i += 3)
    {
      const auto& v0 = vertices[i].position;
      const auto& v1 = vertices[i + 1].position;
      const auto& v2 = vertices[i + 2].position;
      BVHConstructionNode node;
      node.aabb.expand(v0).expand(v1).expand(v2);
      node.center = node.aabb.center();
      nodes.emplace_back(std::move(node));
    }
  }





  BVH bvh;
  return bvh;
}

}

}
