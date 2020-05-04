#pragma once

#include <glm/glm.hpp>
#include <vector>

#include <albedo/instance.h>

namespace albedo
{

using GeometryInstance = Instance;

// TODO: use builder pattern to create geometries?
class Geometry
{
  public:

    Geometry(
      std::vector<glm::vec3>&& _vertices,
      std::vector<glm::vec3>&& _normals,
      std::vector<size_t>&& _indices
    ) noexcept;

    Geometry(Geometry&&) = delete;
    Geometry& operator=(Geometry&&) = delete;

    Geometry(Geometry const&) = delete;
    Geometry& operator=(Geometry const&) = delete;

  public:

    GeometryInstance
    inline getInstance() { return this->m_Instance; }

  private:
    GeometryInstance m_Instance;

    std::vector<glm::vec3> _vertices;
    std::vector<glm::vec3> _normals;
    std::vector<size_t> _indices;

};

class Mesh
{

  public:

    Mesh&
    addGeometry(Geometry&& geom);

};

} // nanespace albedo
