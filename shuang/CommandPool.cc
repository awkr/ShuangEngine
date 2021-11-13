#include "CommandPool.h"
#include "Device.h"
#include "Logger.h"
#include "Macros.h"

CommandPool::CommandPool(const Device &device, uint32_t queueFamilyIndex) : mDevice{device} {
  VkCommandPoolCreateFlags commandPoolCreateFlags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
  VkCommandPoolCreateInfo  commandPoolCreateInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
  commandPoolCreateInfo.flags            = commandPoolCreateFlags;

  vkOK(vkCreateCommandPool(device.getHandle(), &commandPoolCreateInfo, nullptr, &mHandle));
}

CommandPool::~CommandPool() {
  log_func;
  vkDestroyCommandPool(mDevice.getHandle(), mHandle, nullptr);
}
