#include <albedo/mesh.h>

namespace albedo
{

Mesh::Mesh(VertexBuffer&& vb) noexcept
    : m_vertices(std::move(vb))
{ }

Mesh&
Mesh::addPrimitive(Primitive&& primitive)
{
  m_primitives.emplace_back(std::move(primitive));
  return *this;
}

} // namespace albedo
