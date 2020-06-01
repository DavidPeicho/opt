#pragma once

#include <glm/glm.hpp>
#include <albedo/components/component-structure.h>
#include <albedo/mesh.h>

namespace albedo
{

struct Transform
{
  glm::mat4 modelToLocal;
  glm::mat4 localToWorld;
  Mesh::IndexType parentIndex;
  bool isDirty;
};

class TransformManager
{

  public:
  

    void
    computeWorldTransforms();

  private:

    ComponentArray<Transform> m_components;

};

}
