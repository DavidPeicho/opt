#include <albedo/mesh.h>

namespace albedo
{

Mesh::Mesh(VertexBuffer&& vb) noexcept
    : m_vertices(std::move(vb))
{ }

Mesh&
Mesh::setIndices(IndexBuffer&& indices)
{
  m_indices = std::move(indices);
  return *this;
}

} // namespace albedo
