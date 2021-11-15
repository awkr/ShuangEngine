#include "Device.h"
#include "Log.h"
#include "Macros.h"
#include "Queue.h"
#include "Surface.h"

#include <vulkan/vulkan_beta.h>

#define FENCE_DEFAULT_TIMEOUT 100000000000 // Fence default timeout in nanoseconds

Device::Device(const std::shared_ptr<PhysicalDevice> &physicalDevice,
               const std::shared_ptr<Surface>        &surface)
    : mPhysicalDevice{physicalDevice} {
  // Queue family properties, used for setting up requested queues upon device creation
  uint32_t queueFamilyCount;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice->getHandle(), &queueFamilyCount, nullptr);
  mQueueFamilyProperties.resize(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice->getHandle(), &queueFamilyCount,
                                           mQueueFamilyProperties.data());

  // Get list of supported extensions
  uint32_t extensionCount = 0;
  vkEnumerateDeviceExtensionProperties(physicalDevice->getHandle(), nullptr, &extensionCount,
                                       nullptr);
  if (extensionCount > 0) {
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkOK(vkEnumerateDeviceExtensionProperties(physicalDevice->getHandle(), nullptr,
                                                  &extensionCount, extensions.data()));
    //    log_info("Supported device extensions: {}", extensionCount);
    for (const auto &extension : extensions) {
      //      log_info("  {}", extension.extensionName);
      mSupportedExtensions.emplace_back(extension.extensionName);
    }
  }

  // Create device

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
  const float                          defaultQueuePriority = 0.0f;

  // Graphics queue
  int queueFamilyIndex = -1;
  for (auto i = 0; i < mQueueFamilyProperties.size(); ++i) {
    VkBool32 supportPresent;
    if (vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice->getHandle(), i, surface->getHandle(),
                                             &supportPresent);
        !supportPresent) {
      continue;
    }
    if ((mQueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
      queueFamilyIndex = i;
      break;
    }
  }
  if (queueFamilyIndex == -1) {
    throw std::runtime_error("Did not find suitable queue which supports "
                             "graphics and presentation.");
  }
  mQueueFamilyIndices.graphics = queueFamilyIndex;

  VkDeviceQueueCreateInfo queueCreateInfo{};
  queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo.queueFamilyIndex = mQueueFamilyIndices.graphics;
  queueCreateInfo.queueCount       = 1;
  queueCreateInfo.pQueuePriorities = &defaultQueuePriority;
  queueCreateInfos.push_back(queueCreateInfo);

  std::vector<const char *> deviceExtensions{
      VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
      /* VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME, */
  };

  // Check extensions supportability
  for (const auto &extension : deviceExtensions) {
    if (!supportsExtension(extension)) {
      throw std::runtime_error(fmt::format("Device extension not supported: {}", extension));
    }
  }

  VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures{
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT};
  extendedDynamicStateFeatures.extendedDynamicState = VK_TRUE;

  VkDeviceCreateInfo deviceCreateInfo{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
  deviceCreateInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
  deviceCreateInfo.pQueueCreateInfos       = queueCreateInfos.data();
  deviceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size());
  deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
  deviceCreateInfo.pNext                   = &extendedDynamicStateFeatures;

  vkOK(vkCreateDevice(physicalDevice->getHandle(), &deviceCreateInfo, nullptr, &mHandle));

  vkGetDeviceQueue(mHandle, mQueueFamilyIndices.graphics, 0, &mGraphicsQueue);

  mCommandPool = std::make_unique<CommandPool>(*this, mQueueFamilyIndices.graphics);
}

Device::~Device() {
  log_func;
  mCommandPool.reset();
  vkDestroyDevice(mHandle, nullptr);
}

VkCommandBuffer Device::createCommandBuffer(VkCommandBufferLevel level, bool begin) {
  VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
  commandBufferAllocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  commandBufferAllocateInfo.commandPool        = mCommandPool->getHandle();
  commandBufferAllocateInfo.level              = level;
  commandBufferAllocateInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkOK(vkAllocateCommandBuffers(mHandle, &commandBufferAllocateInfo, &commandBuffer));

  // If requested, also start recording for the new command buffer
  if (begin) {
    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkOK(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
  }

  return commandBuffer;
}

void Device::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free,
                                VkSemaphore signalSemaphore) {
  vkOK(vkEndCommandBuffer(commandBuffer));

  VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &commandBuffer;
  if (signalSemaphore) {
    submitInfo.pSignalSemaphores    = &signalSemaphore;
    submitInfo.signalSemaphoreCount = 1;
  }

  // Create fence to ensure that the command buffer has finished executing
  VkFenceCreateInfo fenceCreateInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
  VkFence           fence;
  vkOK(vkCreateFence(mHandle, &fenceCreateInfo, nullptr, &fence));

  // Submit to the queue
  vkOK(vkQueueSubmit(queue, 1, &submitInfo, fence));
  // Wait for the fence to signal that command buffer has finished executing
  vkOK(vkWaitForFences(mHandle, 1, &fence, VK_TRUE, FENCE_DEFAULT_TIMEOUT));

  vkDestroyFence(mHandle, fence, nullptr);

  if (mCommandPool && free) {
    vkFreeCommandBuffers(mHandle, mCommandPool->getHandle(), 1, &commandBuffer);
  }
}

bool Device::supportsExtension(const std::string &extension) {
  return std::find(mSupportedExtensions.begin(), mSupportedExtensions.end(), extension) !=
         mSupportedExtensions.end();
}
