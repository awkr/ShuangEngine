#pragma once

#include <vulkan/vulkan.hpp>

class Device;

class DescriptorPool {
public:
  DescriptorPool(const std::shared_ptr<Device> &device, uint32_t descriptorCount, uint32_t maxSets);
  ~DescriptorPool();

  [[nodiscard]] const VkDescriptorPool &getHandle() const { return mHandle; }

private:
  const std::shared_ptr<Device> &mDevice;
  VkDescriptorPool               mHandle;
};
