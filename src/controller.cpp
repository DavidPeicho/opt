#include <albedo/controller.h>

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace albedo
{

CameraController::CameraController(glm::vec3 origin, glm::vec3 target)
  : m_origin{origin}
  , m_target{target}
  , m_up{glm::vec3(0.0, 1.0, 0.0)}
  , m_right{glm::vec3(1.0, 0.0, 0.0)}
  , m_moveDamping{1.0}
{ }

FPSCameraController::FPSCameraController() noexcept
  : m_localVelocity{glm::vec4(0.0)}
{ }

void
FPSCameraController::update(float deltaTime)
{
  static constexpr glm::vec3 UP_VECTOR(0.0, 1.0, 0.0);

  auto transform = glm::lookAt(m_origin, m_target, UP_VECTOR);
  auto worldVelocity = glm::vec3(transform * m_localVelocity);

  m_origin += worldVelocity;
  m_target += worldVelocity;

  std::cout << worldVelocity.z << std::endl;

  m_localVelocity.x -= m_localVelocity.x * m_moveDamping;
  m_localVelocity.y -= m_localVelocity.y * m_moveDamping;
  m_localVelocity.z -= m_localVelocity.z * m_moveDamping;

  // TOD: prevent direction to align with UP_VECTOR.
  auto direction = getDirection();
  m_right = glm::normalize(glm::cross(direction, UP_VECTOR));
  m_up = glm::normalize(glm::cross(m_right, direction));
}

void
FPSCameraController::forward(float distance)
{
  m_localVelocity.z -= distance;
}

void
FPSCameraController::backward(float distance)
{
  m_localVelocity.z += distance;
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
