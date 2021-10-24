#pragma once

#include "Instance.h"

class PhysicalDevice {
public:
  explicit PhysicalDevice(const Instance *instance);

  const VkPhysicalDevice &getHandle() const { return mPhysicalDevice; }

private:
  VkPhysicalDevice mPhysicalDevice;
};
