#include "FreeCamera.h"
#include "InputEvent.h"
#include "Log.h"

#include <glm/gtx/string_cast.hpp>

FreeCamera::FreeCamera(const glm::vec3 &position) : Camera(position) {}

FreeCamera::~FreeCamera() { log_func; }

void FreeCamera::update(float timeStep) {
  Camera::update(timeStep);

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

  if (mKeyPressed[KeyCode::Q]) {
    rotation.z += mRotateSpeed;
  }
  if (mKeyPressed[KeyCode::E]) {
    rotation.z -= mRotateSpeed;
  }

  if (mMouseButtonPressed[MouseButton::LEFT]) {
    rotation.x += 0.15f * mMouseMoveDelta.y;
    rotation.y += 0.15f * -mMouseMoveDelta.x;
  }

  rotation *= timeStep;

  {
    auto qz = glm::angleAxis(rotation.z, mForward);
    mRight  = glm::normalize(qz * mRight);
    mUp     = glm::normalize(qz * mUp);
  }

  {
    auto qx  = glm::angleAxis(rotation.x, mRight);
    mForward = glm::normalize(qx * mForward);

    if (glm::dot(mUp, glm::cross(mRight, mForward)) < 0) {
      mUp *= -1;
    }
  }

  {
    auto qy  = glm::angleAxis(rotation.y, mUp);
    mForward = glm::normalize(qy * mForward);
    mRight   = glm::normalize(qy * mRight);
  }

  mRotation = glm::quatLookAt(mForward, mUp);

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
    movement.x += 0.4f * mMouseMoveDelta.x;
    movement.y += 0.4f * mMouseMoveDelta.y;
  }
  movement.z += 1.25f * mMouseScrollDelta;

  movement *= timeStep;

  mPosition += movement * glm::conjugate(mRotation);

  if (mKeyPressed[KeyCode::F5]) {
    resetTransform();
  }

  updateViewMatrix();

  mMouseMoveDelta   = {0.0f, 0.0f};
  mMouseScrollDelta = 0.0f;
}

void FreeCamera::resetTransform() {
  mPosition = glm::vec3(0.0f, 0.0f, -3.0f);

  mForward = glm::vec3{0.0f, 0.0f, 1.0f};
  mUp      = glm::vec3{0.0f, -1.0f, 0.0f};
  mRight   = glm::vec3{1.0f, 0.0f, 0.0f};

  mRotation = glm::quatLookAt(mForward, mUp);
}
