#pragma once

#include <vulkan/vulkan.hpp>

#include "PhysicalDevice.h"

class Surface;

class Device {
public:
  struct QueueFamilyIndices {
    uint32_t graphics;
    uint32_t compute;
    uint32_t transfer;
  };

  Device(const std::shared_ptr<PhysicalDevice> &physicalDevice,
         const std::shared_ptr<Surface>        &surface);
  ~Device();

  const VkDevice                        &getHandle() const { return mHandle; }
  const std::shared_ptr<PhysicalDevice> &getPhysicalDevice() const {
    return mPhysicalDevice;
  }
  const QueueFamilyIndices &getQueueFamilyIndices() const {
    return mQueueFamilyIndices;
  }
  const VkQueue &getGraphicsQueue() const { return mGraphicsQueue; }
  VkResult       waitIdle() const { return vkDeviceWaitIdle(mHandle); }

private:
  bool isExtensionSupported(const std::string &extension);

  const std::shared_ptr<PhysicalDevice> &mPhysicalDevice = nullptr;
  VkPhysicalDeviceProperties             mPhysicalDeviceProperties;
  VkPhysicalDeviceFeatures               mPhysicalDeviceFeatures;
  VkPhysicalDeviceMemoryProperties       mPhysicalDeviceMemoryProperties;
  std::vector<VkQueueFamilyProperties>   mQueueFamilyProperties;
  std::vector<std::string>               mSupportedExtensions;
  QueueFamilyIndices                     mQueueFamilyIndices;
  VkDevice                               mHandle;
  VkQueue                                mGraphicsQueue;
};
