#include <albedo/mesh.h>

namespace albedo
{

Mesh::Mesh(VertexBuffer&& vb) noexcept
    : m_vertices(std::move(vb))
    , m_nbTriangles(0)
{ }

Mesh&
Mesh::addPrimitive(Primitive&& primitive)
{
  m_nbTriangles += primitive.size() / 3;
  m_primitives.emplace_back(std::move(primitive));
  return *this;
}

} // namespace albedo
