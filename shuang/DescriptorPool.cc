#include "DescriptorPool.h"
#include "Device.h"
#include "Logger.h"
#include "Macros.h"

DescriptorPool::DescriptorPool(const std::shared_ptr<Device> &device, uint32_t descriptorCount,
                               uint32_t maxSets)
    : mDevice{device} {
  std::vector<VkDescriptorPoolSize> descriptorPoolSizes{};

  VkDescriptorPoolSize descriptorPoolSize;
  descriptorPoolSize.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptorPoolSize.descriptorCount = descriptorCount;

  descriptorPoolSizes.push_back(descriptorPoolSize);

  VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
  descriptorPoolCreateInfo.pPoolSizes    = descriptorPoolSizes.data();
  descriptorPoolCreateInfo.maxSets       = maxSets;

  vkOK(
      vkCreateDescriptorPool(device->getHandle(), &descriptorPoolCreateInfo, nullptr, &mHandle));
}

DescriptorPool::~DescriptorPool() {
  log_func;
  vkDestroyDescriptorPool(mDevice->getHandle(), mHandle, nullptr);
}
