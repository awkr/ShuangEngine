#pragma once

#include <unordered_map>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class InputEvent;
enum class KeyCode;
enum class MouseButton;

// the perspective free fly camera
class Camera {
public:
  explicit Camera(glm::vec3 position = {0.0f, 0.0f, -3.0f});
  ~Camera();

  [[nodiscard]] const glm::mat4 &getViewMatrix() const { return mViewMatrix; }
  [[nodiscard]] const glm::mat4 &getProjectionMatrix() { return mProjectionMatrix; }
  [[nodiscard]] const glm::vec3 &getPosition() const { return mPosition; }
  [[nodiscard]] const glm::quat &getRotation() const { return mRotation; }

  // fov: in degrees
  void setPerspective(float fov, float aspect, float zNear, float zFar);

  void handleEvent(const InputEvent &inputEvent);
  void update(float timeStep);

private:
  void updateViewMatrix();
  void updateProjectionMatrix();
  // Reset position and rotation
  void resetTransform();

  float mFov{50.0f};
  float mZNear{0.1f};
  float mZFar{100.0f};
  float mAspect{1.0f};

  glm::vec3 mPosition{0.0f, 0.0f, -3.0f};
  glm::quat mRotation{1.0f, 0.0f, 0.0f, 0.0f};
  glm::mat4 mViewMatrix;
  glm::mat4 mProjectionMatrix;

  // Control
  float mRotateSpeed = 1.2f;
  float mMoveSpeed   = 1.8f;

  std::unordered_map<KeyCode, bool>     mKeyPressed;
  std::unordered_map<MouseButton, bool> mMouseButtonPressed;

  glm::vec3 mForward{0.0f, 0.0f, 1.0f};
  // In Vulkan, Y direction is downstairs
  glm::vec3 mUp{0.0f, -1.0f, 0.0f};
  glm::vec3 mRight{1.0f, 0.0f, 0.0f};
};
