#pragma once

#include "Model.h"

class Cube : public Model {
public:
  explicit Cube(const std::shared_ptr<Device> &device);
  ~Cube();
};
