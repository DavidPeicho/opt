#include <albedo/accel/gpu-accel.h>
#include <albedo/accel/bvh-sah-builder.h>

namespace albedo
{

namespace accel
{

namespace
{

}

void
GPUAccelerationStructure::addMeshes(const std::vector<Mesh::MeshPtr>& meshes)
{
  // TODO: replace by a mesh to BVH tracking.
  for (const auto& m: meshes) { m_meshes.emplace_back(m); }
}

void
GPUAccelerationStructure::build(const Scene& scene)
{
  // TODO: move somewhere else. Shouldnt be created and destroyed.
  std::vector<BVH> bvhs(m_meshes.size());

  size_t totalNumberNodes = 0;
  for (const auto& m: m_meshes)
  {
    accel::SAHBuilder builder;
    auto bvh = builder.build(*m);
    totalNumberNodes += bvh.getNodesCount();
    bvhs.emplace_back(std::move(bvh));
  }

  m_nodes.clear();
  m_nodes.reserve(totalNumberNodes);
  for (size_t i = 0; i < bvhs.size(); ++i)
  {
  }

}

} // namespace accel

} // namespace albedo
