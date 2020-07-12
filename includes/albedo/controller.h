#pragma once

#include <glm/glm.hpp>

namespace albedo
{

class CameraController
{
  public:

    CameraController(
      glm::vec3 origin,
      glm::vec3 target
    );

    virtual ~CameraController() { }

  public:

    virtual void
    update(float deltaTime) = 0;

    // TODO: add more generic control method, such as `grab`, `move`, etc...
    // It's pretty common to change controls at runtime.

  public:

    inline void
    setOrigin(const glm::vec3& origin) { m_origin = origin; }

    inline const glm::vec3&
    getOrigin() const { return m_origin; }

    inline const glm::vec3
    getDirection() const { return glm::normalize(m_target - m_origin); }

    inline const glm::vec3&
    getRight() const { return m_right; }

    inline const glm::vec3&
    getUp() const { return m_up; }

    inline const bool
    isDirty() const { return m_dirty; }

  protected:

    glm::vec3 m_origin;
    glm::vec3 m_target;
    glm::vec3 m_up;
    glm::vec3 m_right;

    // TODO: a lot of parameters are actually dependant on the controller
    // type used.
    float m_moveSpeed;
    float m_rotSpeed;
    float m_moveDamping;
    float m_rotationDamping;

    bool m_dirty;
};

class FPSCameraController: public CameraController
{

  public:

    FPSCameraController(
      glm::vec3 origin = glm::vec3(0.0, 0.0, 1.0),
      glm::vec3 target = glm::vec3(0.0)
    ) noexcept;

  public:

    virtual void
    update(float deltaTime = 1.0) override;

  public:

    void
    rotate(const glm::vec2& direction);

    void
    forward(float distance = 1.0);

    void
    backward(float distance = 1.0);

    void
    left(float distance = 1.0);

    void
    right(float distance = 1.0);

  private:

    glm::vec3 m_localVelocity;
    glm::vec2 m_localRotVelocity;

};

} // namespace albedo
