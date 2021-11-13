#pragma once

#include "Model.h"

class Triangle : public Model {
public:
  explicit Triangle(const std::shared_ptr<Device> &device);
  ~Triangle();
};
