#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <albedo/instance.h>

namespace albedo
{

struct Vertex
{
  glm::vec3 position;
  glm::vec3 normal;
  // TODO: add UV
};

struct Geometry
{
  using IndexType = uint32_t;

  std::vector<IndexType> indices;
};

// TODO: add support for material per VB
class Mesh
{
  public:

    using VertexBuffer = std::vector<Vertex>;

  public:

    Mesh() noexcept = default;

    Mesh(Mesh&&) noexcept = default;

    Mesh(VertexBuffer&& vb) noexcept;

    Mesh(Mesh const&) = delete;
    Mesh& operator=(Mesh const&) = delete;

  public:

    inline const std::string&
    name() const { return m_name; }

    inline const std::string&
    name(const std::string& name) { m_name = name; return m_name; }

    inline const std::string&
    name(std::string&& name) { m_name = std::move(name); return m_name; }

    Mesh&
    addGeometry(Geometry&& geometry);

    inline VertexBuffer&
    getVertexBuffer() { return m_vertices; }

    inline const VertexBuffer&
    getVertexBuffer() const { return m_vertices; }

    inline const std::vector<Geometry>&
    getGeometries() const { return m_geometries; }

    inline Geometry::IndexType
    getTrianglesCount() const { return m_nbTriangles; }

  private:
    std::string m_name;
    VertexBuffer m_vertices;
    std::vector<Geometry> m_geometries;

    Geometry::IndexType m_nbTriangles;
};

} // nanespace albedo
