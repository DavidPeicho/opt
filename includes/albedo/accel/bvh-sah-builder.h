#include <albedo/accel/bvh.h>
#include <albedo/mesh.h>

namespace albedo
{

namespace accel
{

class SAHBuilder
{

  friend BVH;

  public:

    BVH
    build(const Mesh&);

};

}

}
