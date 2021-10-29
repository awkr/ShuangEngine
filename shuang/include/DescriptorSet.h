#pragma once

#include <vulkan/vulkan.hpp>

class Device;
class DescriptorPool;

class DescriptorSet {
public:
  DescriptorSet(const std::shared_ptr<Device> &device, const std::shared_ptr<DescriptorPool> &pool,
                uint32_t                                  descriptorSetCount,
                const std::vector<VkDescriptorSetLayout> &descriptorSetLayouts,
                const VkDescriptorBufferInfo             &bufferInfo);

  [[nodiscard]] const VkDescriptorSet &getHandle() const { return mHandle; }

private:
  static VkWriteDescriptorSet createWriteDescriptorSet(VkDescriptorSet  dstSet,
                                                       VkDescriptorType type, uint32_t sdtBinding,
                                                       const VkDescriptorBufferInfo *bufferInfo,
                                                       uint32_t descriptorCount = 1);

  VkDescriptorSet mHandle = VK_NULL_HANDLE;
};
