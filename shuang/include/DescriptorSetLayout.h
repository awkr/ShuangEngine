#pragma once

#include <vulkan/vulkan.hpp>

class Device;

class DescriptorSetLayout {
public:
  explicit DescriptorSetLayout(const std::shared_ptr<Device> &device);
  ~DescriptorSetLayout();

  const VkDescriptorSetLayout &getHandle() const { return mHandle; }

private:
  static VkDescriptorSetLayoutBinding createDescriptorSetLayoutBinding(VkDescriptorType   type,
                                                                       VkShaderStageFlags stages,
                                                                       uint32_t           binding,
                                                                       uint32_t count = 1);

  const std::shared_ptr<Device> &mDevice;
  VkDescriptorSetLayout          mHandle{VK_NULL_HANDLE};
};
