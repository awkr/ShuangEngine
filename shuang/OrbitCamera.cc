#include "OrbitCamera.h"
#include "InputEvent.h"
#include "Logger.h"

#include <glm/gtx/string_cast.hpp>

OrbitCamera::OrbitCamera(const glm::vec3 &position, const glm::vec3 &target) : Camera(position) {
  const auto forward = glm::normalize(target - position);
  auto       up      = glm::vec3(0.0f, -1.0f, 0.0f);
  const auto right   = glm::cross(forward, up);
  up                 = glm::cross(right, forward);
  mRotation          = glm::quatLookAt(forward, up);

  updateViewMatrix();
}

OrbitCamera::~OrbitCamera() { log_func; }

void OrbitCamera::update(float timeStep) {
  // Rotation
  glm::vec3 rotation{0.0f};

  if (mKeyPressed[KeyCode::UP]) {
    rotation.x -= mRotateSpeed;
  }
  if (mKeyPressed[KeyCode::DOWN]) {
    rotation.x += mRotateSpeed;
  }

  if (mKeyPressed[KeyCode::LEFT]) {
    rotation.y += mRotateSpeed;
  }
  if (mKeyPressed[KeyCode::RIGHT]) {
    rotation.y -= mRotateSpeed;
  }

  //  if (mKeyPressed[KeyCode::Q]) {
  //    rotation.z += mRotateSpeed;
  //  }
  //  if (mKeyPressed[KeyCode::E]) {
  //    rotation.z -= mRotateSpeed;
  //  }

  if (mMouseButtonPressed[MouseButton::LEFT]) {
    rotation.x += 0.4f * mMouseMoveDelta.y;
    rotation.y -= 0.4f * mMouseMoveDelta.x;
  }

  rotation *= timeStep;

  auto up = glm::vec3(0, -1, 0);
  //  if (glm::dot(up, mRotation * up) < 0) {
  //    up *= -1;
  //  }
  const auto right = mRotation * glm::vec3(1, 0, 0);

  const auto qy = glm::angleAxis(rotation.y, up);
  const auto qx = glm::angleAxis(rotation.x, right);
  const auto q  = glm::normalize(qy * qx);

  mPosition = q * mPosition;
  mRotation = q * mRotation;

  // Movement

  glm::vec3 movement{0.0f};

  if (mKeyPressed[KeyCode::A]) {
    movement.x -= mMoveSpeed;
  }
  if (mKeyPressed[KeyCode::D]) {
    movement.x += mMoveSpeed;
  }

  if (mKeyPressed[KeyCode::W]) {
    movement.z -= mMoveSpeed;
  }
  if (mKeyPressed[KeyCode::S]) {
    movement.z += mMoveSpeed;
  }

  if (mKeyPressed[KeyCode::R]) {
    movement.y -= mMoveSpeed;
  }
  if (mKeyPressed[KeyCode::F]) {
    movement.y += mMoveSpeed;
  }

  if (mMouseButtonPressed[MouseButton::RIGHT]) {
    movement.x -= 0.75f * mMouseMoveDelta.x;
    movement.y -= 0.75f * mMouseMoveDelta.y;
  }
  movement.z -= 1.2f * mMouseScrollDelta;

  movement *= timeStep;

  //  mPosition += movement * glm::conjugate(mRotation);
  //  mPosition += movement;             // move along the world space axis
  mPosition += mRotation * movement; // move along the camera space axis

  if (mKeyPressed[KeyCode::F5]) {
    resetTransform();
  }

  updateViewMatrix();

  mMouseMoveDelta   = {0.0f, 0.0f};
  mMouseScrollDelta = 0.0f;
}

void OrbitCamera::resetTransform() {}
