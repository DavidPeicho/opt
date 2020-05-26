#include <array>

#include <albedo/accel/bvh.h>
#include <albedo/mesh.h>

namespace albedo
{

namespace accel
{

namespace
{

struct Bin
{
  math::Box3 aabb;
  Mesh::IndexType nbPrimitives;
  float rightCost;
};

}

template <Mesh::IndexType BinCount = 12>
class SAHBuilder
{

  public:

    SAHBuilder(BVH& bvh);

  public:

    void
    build(const Mesh&);

  private:

    BVH& m_bvh;
    std::array<Bin, BinCount> m_bins;

};

}

}

#include <albedo/accel/bvh-sah-builder.hxx>
