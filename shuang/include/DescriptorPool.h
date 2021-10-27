#pragma once

#include <vulkan/vulkan.hpp>

class Device;

class DescriptorPool {
public:
  DescriptorPool(const std::shared_ptr<Device> &device);
  ~DescriptorPool();

private:
  const std::shared_ptr<Device> &mDevice;
  VkDescriptorPool               mHandle;
};
