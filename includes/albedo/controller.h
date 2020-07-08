#pragma once

#include <glm/glm.hpp>

namespace albedo
{

class CameraController
{
  public:

    CameraController(
      glm::vec3 origin = glm::vec3(0.0, 0.0, 1.0),
      glm::vec3 target = glm::vec3(0.0)
    );

    virtual ~CameraController() { }

  public:

    virtual void
    update(float deltaTime) = 0;

  public:

    inline const glm::vec3&
    getOrigin() const { return m_origin; }

    inline const glm::vec3
    getDirection() const { return glm::normalize(m_target - m_origin); }

    inline const glm::vec3&
    getRight() const { return m_right; }

    inline const glm::vec3&
    getUp() const { return m_up; }

  protected:

    glm::vec3 m_origin;
    glm::vec3 m_target;
    glm::vec3 m_up;
    glm::vec3 m_right;

    float m_moveDamping;
};

class FPSCameraController: public CameraController
{

  public:

    FPSCameraController() noexcept;

  public:

    virtual void
    update(float deltaTime = 1.0) override;

  public:

    void
    forward(float distance = 1.0);

    void
    backward(float distance = 1.0);

    void
    left(float distance = 1.0);

    void
    right(float distance = 1.0);

  private:

    glm::vec4 m_localVelocity;

};

} // namespace albedo
