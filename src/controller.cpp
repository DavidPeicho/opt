#include <albedo/controller.h>

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>


// DEBUG
#include <glm/gtx/string_cast.hpp>
#include <iostream>

namespace albedo
{

CameraController::CameraController(glm::vec3 origin, glm::vec3 target)
  : m_origin{origin}
  , m_target{target}
  , m_up{glm::vec3(0.0, 1.0, 0.0)}
  , m_right{glm::vec3(1.0, 0.0, 0.0)}
  , m_moveSpeed{1.0}
  , m_rotSpeed{1.0}
  , m_moveDamping{0.5}
  , m_rotationDamping{0.5}
  , m_currMoveDamping{0.0}
  , m_currRotDamping{0.0}
  , m_dirty{false}
{ }

FPSCameraController::FPSCameraController(
  glm::vec3 origin,
  glm::vec3 target) noexcept
  : CameraController(origin, target)
  , m_moveLeft{false}
  , m_moveRight{false}
  , m_moveForward{false}
  , m_moveBackward{false}
  , m_velocity{glm::vec3(0.0)}
  , m_angularVelocity{glm::vec2(0.0)}
{ }

void
FPSCameraController::update(float deltaTime)
{
  static constexpr glm::vec3 UP_VECTOR(0.0, 1.0, 0.0);
  static constexpr glm::vec3 ZERO_VECTOR3(0.0, 0.0, 0.0);
  static constexpr glm::vec2 ZERO_VECTOR2(0.0, 0.0);

  if (m_moveForward) { m_velocity.z += m_moveSpeed; }
  if (m_moveBackward) { m_velocity.z -= m_moveSpeed; }
  if (m_moveRight) { m_velocity.x += m_moveSpeed; }
  if (m_moveLeft) { m_velocity.x -= m_moveSpeed; }

  m_dirty = glm::any(glm::epsilonNotEqual(m_velocity, ZERO_VECTOR3, 0.0001f))
            || glm::any(glm::epsilonNotEqual(m_angularVelocity, ZERO_VECTOR2, 0.0001f));

  if (!m_dirty) { return; }

  // Computes new rotation.
  auto rotVel = m_angularVelocity * m_rotSpeed * deltaTime ;
  auto rot = glm::angleAxis(- rotVel.x, m_up)
            * glm::angleAxis(- rotVel.y, m_right);

  auto direction = getDirection();
  direction = glm::normalize(rot * direction);

  m_right = glm::normalize(glm::cross(direction, UP_VECTOR));
  m_up = glm::normalize(glm::cross(m_right, direction));

  // Computes new position.
  auto forceWorld = m_velocity.x * m_right + m_velocity.z * direction;
  m_origin += forceWorld * m_moveSpeed * deltaTime;
  m_target = m_origin + direction;

  // Damping
  float invDelta = (1.0f / deltaTime);
  m_velocity *= 1.0 - glm::clamp(m_moveDamping ,0.05f, 0.99f);
  m_angularVelocity *= glm::clamp(
    (1.0f - m_rotationDamping) * (1.0f - invDelta),
    0.0f, 1.0f
  );

  m_moveForward = false;
  m_moveBackward = false;
  m_moveRight = false;
  m_moveLeft = false;
}

void
FPSCameraController::rotate(const glm::vec2& direction)
{
  m_angularVelocity += direction;
}

void
FPSCameraController::forward()
{
  m_moveForward = true;
}

void
FPSCameraController::backward()
{
  m_moveBackward = true;
}

void
FPSCameraController::left()
{
  m_moveLeft = true;
}

void
FPSCameraController::right()
{
  m_moveRight = true;
}

} // namespace albedo
