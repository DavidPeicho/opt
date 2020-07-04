#pragma once

#include <glm/glm.hpp>
#include <albedo/components/component.h>
#include <albedo/components/component-structure.h>
#include <albedo/entity.h>

namespace albedo
{

class Scene;

namespace components
{

struct Light : public Component<Light>
{
  Light()
    : intensity(1.0)
    , temperature(100.0)
    , width(1.0)
    , height(1.0)
  { }

  float intensity;
  float temperature;
  float width;
  float height;
};

// TODO: make a base manager class. No need to make dynamic dispatch though,
// there is no logic in having manager with polymorphism.
class LightManager
{

  public:

    using LightId = typename Light::Id;

    friend Scene;

  public:

    LightManager&
    createComponent(const Entity&);

    LightManager&
    createComponent(const Entity&, Light&&);

  private:

    ComponentArray<Light> m_components;

};

} // namespace components

} // namespace albedo
