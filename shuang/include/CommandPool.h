#pragma once

#include <vulkan/vulkan.hpp>

class Device;

class CommandPool {
public:
  CommandPool(const Device &device, uint32_t queueFamilyIndex);
  ~CommandPool();

  [[nodiscard]] const VkCommandPool &getHandle() const { return mHandle; }

private:
  const Device &mDevice;
  VkCommandPool mHandle = VK_NULL_HANDLE;
};
