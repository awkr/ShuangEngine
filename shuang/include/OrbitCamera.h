#pragma once

#include "Camera.h"

// the perspective camera which can orbit around a target
class OrbitCamera : public Camera {
public:
  OrbitCamera(const glm::vec3 &position = {1.0f, 2.0f, -2.0f},
              const glm::vec3 &target   = {0.0f, 0.0f, 0.0f});
  ~OrbitCamera();

  void update(float timeStep) override;

protected:
  void resetTransform() override;

private:
};
