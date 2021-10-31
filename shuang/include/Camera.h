#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

class Camera {
public:
  enum CameraType { LOOK_AT, FIRST_PERSON };

  ~Camera();

  const glm::mat4 &getViewMatrix() {}
  const glm::mat4 &getProjectionMatrix() {}

  void setType(const CameraType type) { mType = type; }

  void setPosition(glm::vec3 position) {
    mPosition = position;
    updateViewMatrix();
  }

  void setPerspective(float fov, float aspect, float zNear, float zFar) {
    mFov   = fov;
    mZNear = zNear;
    mZFar  = zFar;
  }

  void setRotation(glm::vec3 rotation) {
    mRotation = rotation;
    updateViewMatrix();
  }

private:
  void updateViewMatrix() {}

  CameraType mType     = CameraType::LOOK_AT;
  glm::vec3  mPosition = glm::vec3();
  glm::vec3  mRotation = glm::vec3();
  float      mFov;
  float      mZNear;
  float      mZFar;
  float      mRotateSpeed = 0.1f;
  float      mMoveSpeed   = 1.0f;
};
