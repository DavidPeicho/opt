#include <albedo/controller.h>

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
  , m_moveSpeed{0.1}
  , m_rotSpeed{1.0}
  , m_moveDamping{15.0}
  , m_rotationDamping{8.0}
  , m_dirty{false}
{ }

FPSCameraController::FPSCameraController(
  glm::vec3 origin,
  glm::vec3 target) noexcept
  : CameraController(origin, target)
  , m_localVelocity{glm::vec3(0.0)}
  , m_localRotVelocity{glm::vec2(0.0)}
{ }

void
FPSCameraController::update(float deltaTime)
{
  static constexpr glm::vec3 UP_VECTOR(0.0, 1.0, 0.0);
  static constexpr glm::vec3 ZERO_VECTOR3(0.0, 0.0, 0.0);
  static constexpr glm::vec2 ZERO_VECTOR2(0.0, 0.0);

  m_dirty = glm::any(glm::epsilonNotEqual(m_localVelocity, ZERO_VECTOR3, 0.0001f))
            || glm::any(glm::epsilonNotEqual(m_localRotVelocity, ZERO_VECTOR2, 0.0001f));

  if (!m_dirty) { return; }

  // Computes new rotation.
  auto rotVel = m_localRotVelocity * m_rotationDamping * deltaTime;
  auto rot = glm::angleAxis(- rotVel.x * m_rotSpeed, m_up)
            * glm::angleAxis(- rotVel.y * m_rotSpeed, m_right);

  auto direction = getDirection();
  direction = glm::normalize(rot * direction);

  m_right = glm::normalize(glm::cross(direction, UP_VECTOR));
  m_up = glm::normalize(glm::cross(m_right, direction));

  // Computes new position.
  auto moveVel = m_localVelocity * m_moveDamping * deltaTime;
  auto worldTranslation = moveVel.x * m_right + moveVel.z * direction;
  worldTranslation *= m_moveSpeed;

  auto test = m_origin;
  m_origin += worldTranslation;
  m_target = m_origin + direction;

  // Damping
  m_localVelocity -= moveVel;
  m_localRotVelocity -= rotVel;
}

void
FPSCameraController::rotate(const glm::vec2& direction)
{
  m_localRotVelocity = direction;
}

void
FPSCameraController::forward(float distance)
{
  m_localVelocity.z += distance;
}

void
FPSCameraController::backward(float distance)
{
  m_localVelocity.z -= distance;
}

void
FPSCameraController::left(float distance)
{
  m_localVelocity.x -= distance;
}

void
FPSCameraController::right(float distance)
{
  m_localVelocity.x += distance;
}

} // namespace albedo
