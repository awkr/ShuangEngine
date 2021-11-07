#pragma once

#include "Camera.h"

// the perspective free fly camera
class FreeCamera : public Camera {
public:
  explicit FreeCamera(const glm::vec3 &position = {0.0f, 0.0f, -3.0f});
  ~FreeCamera();

  void update(float timeStep) override;

protected:
  void resetTransform() override;
};
