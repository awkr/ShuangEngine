#pragma once

#include <vulkan/vulkan.hpp>

class Queue {
public:
  bool                           isSupportPresent() const { return false; }
  const VkQueueFamilyProperties &getProperties() const { return mProperties; }

  VkResult waitIdle() const { return vkQueueWaitIdle(mHandle); }

private:
  VkQueue                 mHandle;
  VkQueueFamilyProperties mProperties;
};
