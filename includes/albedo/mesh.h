#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <albedo/accel/bvh.h>

namespace albedo
{

struct Vertex
{
  glm::vec3 position;
  uint32_t padding_0; // For GPU
  // TODO: add UV
};

// TODO: add support for material per VB
class Mesh
{
  public:

    using MeshPtr = std::shared_ptr<Mesh>;

    using IndexType = uint32_t; // TODO: move out of Mesh class
    using VertexBuffer = std::vector<Vertex>;
    using IndexBuffer = std::vector<IndexType>;

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
    setIndices(IndexBuffer&& buffer);

    inline VertexBuffer&
    getVertexBuffer() { return m_vertices; }

    inline const VertexBuffer&
    getVertexBuffer() const { return m_vertices; }

    inline const IndexBuffer&
    getIndices() const { return m_indices; }

    inline IndexType
    getTrianglesCount() const { return m_indices.size() / 3; }

    inline accel::BVH&
    getBVH() { return m_bvh; }

  private:
    std::string m_name;
    VertexBuffer m_vertices;
    IndexBuffer m_indices;
    accel::BVH m_bvh;
};

} // nanespace albedo
