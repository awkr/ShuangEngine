#pragma once

#include "Model.h"

class Grid : public Model {
public:
  explicit Grid(const std::shared_ptr<Device> &device, int halfSize = 1);
  ~Grid();
};
