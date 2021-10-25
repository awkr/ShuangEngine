#pragma once

#include "Instance.h"

class PhysicalDevice {
public:
  explicit PhysicalDevice(const std::shared_ptr<Instance> &instance);
  ~PhysicalDevice();

  const VkPhysicalDevice &getHandle() const { return mHandle; }

private:
  VkPhysicalDevice mHandle;
};
