#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace albedo
{

namespace scene
{

// TODO: use builder pattern to create geometries?
class Geometry
{
  public:

    Geometry(
      std::vector<glm::vec3>&& _vertices,
      std::vector<glm::vec3>&& _normals,
      std::vector<size_t>&& _indices
    );

    Geometry(Geometry&&) = delete;
    Geometry& operator=(Geometry&&) = delete;

    Geometry(Geometry const&) = delete;
    Geometry& operator=(Geometry const&) = delete;

  private:
    std::vector<glm::vec3> _vertices;
    std::vector<glm::vec3> _normals;
    std::vector<size_t> _indices;

};

} // nanespace scene

} // nanespace albedo
