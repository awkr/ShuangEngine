#include "Camera.h"
#include "InputEvent.h"
#include "Logger.h"

#include <glm/gtx/string_cast.hpp>

Camera::Camera(glm::vec3 position) : mPosition{position} {}

Camera::~Camera() { log_func; }

void Camera::setPerspective(float fov, float aspect, float zNear, float zFar) {
  mFov    = glm::radians(fov);
  mAspect = aspect;
  mZNear  = zNear;
  mZFar   = zFar;

  updateProjectionMatrix();
}

void Camera::handleEvent(const InputEvent &inputEvent) {
  if (inputEvent.getSource() == InputEventSource::KEYBOARD) {
    const auto &event = static_cast<const KeyInputEvent &>(inputEvent);
    if (event.getAction() == KeyAction::DOWN || event.getAction() == KeyAction::REPEAT) {
      mKeyPressed[event.getCode()] = true;
    } else {
      mKeyPressed[event.getCode()] = false;
    }
  } /*  else if (inputEvent.getSource() == InputEventSource::MOUSE) {
    const auto &event = static_cast<const MouseButtonInputEvent &>(inputEvent);

    glm::vec2        pos{std::floor(event.getX()), std::floor(event.getY())};
    static glm::vec2 lastPos{0.f};

    if (event.getAction() == MouseAction::DOWN) {
      mMouseButtonPressed[event.getButton()] = true;
      //      lastPos                                = pos;
    } else if (event.getAction() == MouseAction::UP) {
      mMouseButtonPressed[event.getButton()] = false;
    } else if (event.getAction() == MouseAction::MOVE) {
      mMouseMoveDelta = pos - lastPos;
      lastPos         = pos;
    }
  } */
}

void Camera::update(float timeStep) {
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

  movement *= timeStep;

  mPosition += movement * glm::conjugate(mRotation);

  if (mKeyPressed[KeyCode::F5]) {
    resetTransform();
  }

  updateViewMatrix();
}

void Camera::updateViewMatrix() {
  mViewMatrix = glm::inverse(glm::translate(glm::mat4(1.f), mPosition) * glm::mat4_cast(mRotation));
}

void Camera::updateProjectionMatrix() {
  mProjectionMatrix = glm::perspective(mFov, mAspect, mZNear, mZFar);
}

void Camera::resetTransform() {
  mPosition = glm::vec3(0.0f, 0.0f, -3.0f);

  mForward = glm::vec3{0.0f, 0.0f, 1.0f};
  mUp      = glm::vec3{0.0f, -1.0f, 0.0f};
  mRight   = glm::vec3{1.0f, 0.0f, 0.0f};

  mRotation = glm::quatLookAt(mForward, mUp);
}
