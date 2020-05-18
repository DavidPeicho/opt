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

  friend BVH;

  public:

    BVH
    build(const Mesh&);

  private:

    std::array<Bin, BinCount> m_bins;

};

}

}

#include <albedo/accel/bvh-sah-builder.hxx>
