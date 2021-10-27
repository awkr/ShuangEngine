#include "DescriptorPool.h"
#include "Device.h"
#include "Logger.h"
#include "Macros.h"

DescriptorPool::DescriptorPool(const std::shared_ptr<Device> &device) : mDevice{device} {
  // TODO

  VkDescriptorPoolSize descriptorPoolSize{};
  descriptorPoolSize.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptorPoolSize.descriptorCount = 2;

  VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
  descriptorPoolCreateInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPoolCreateInfo.poolSizeCount = 1;
  descriptorPoolCreateInfo.pPoolSizes    = &descriptorPoolSize;
  descriptorPoolCreateInfo.maxSets       = 2;

  vkAssert(
      vkCreateDescriptorPool(device->getHandle(), &descriptorPoolCreateInfo, nullptr, &mHandle));
}

DescriptorPool::~DescriptorPool() {
  log_func;
  vkDestroyDescriptorPool(mDevice->getHandle(), mHandle, nullptr);
}
