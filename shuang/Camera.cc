#include "Camera.h"
#include "InputEvent.h"

Camera::Camera(const glm::vec3 &position) : mPosition{position} {}

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
  } else if (inputEvent.getSource() == InputEventSource::MOUSE) {
    const auto &event = static_cast<const MouseButtonInputEvent &>(inputEvent);

    if (event.getAction() == MouseAction::SCROLL) {
      mMouseScrollDelta = std::floor(event.getY());
    } else {
      glm::vec2        pos{std::floor(event.getX()), std::floor(event.getY())};
      static glm::vec2 lastPos{0.f};

      if (event.getAction() == MouseAction::DOWN) {
        mMouseButtonPressed[event.getButton()] = true;
        lastPos                                = pos;
      } else if (event.getAction() == MouseAction::UP) {
        mMouseButtonPressed[event.getButton()] = false;
      } else if (event.getAction() == MouseAction::MOVE) {
        mMouseMoveDelta = pos - lastPos;
        lastPos         = pos;
      }
    }
  }
}

void Camera::update(float timeStep) {}

void Camera::updateViewMatrix() {
  mViewMatrix =
      glm::inverse(glm::translate(glm::mat4(1.0f), mPosition) * glm::mat4_cast(mRotation));
}

void Camera::updateProjectionMatrix() {
  mProjectionMatrix = glm::perspective(mFov, mAspect, mZNear, mZFar);
}
