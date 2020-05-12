#include <albedo/accel/bvh-sah-builder.h>


namespace albedo
{

namespace accel
{

BVH
SAHBuilder::build(const Mesh& mesh)
{
  Mesh::IndexType nbTriangles = mesh.getTrianglesCount();

  std::vector<BVHNode> nodes(2 * nbTriangles + 1);

  // TODO: move this into the mesh directly.
  std::vector<math::AABB> boxes;
  boxes.reserve(nbTriangles);
  std::vector<math::AABB> centers;
  centers.reserve(nbTriangles);

  for (const auto& primitive: mesh.getPrimitives())
  {
  }

  BVH bvh;
  return bvh;
}

}

}
