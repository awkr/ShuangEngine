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

  Device(const PhysicalDevice *physicalDevice, const Surface *surface);
  ~Device();

  const VkDevice       &getHandle() const { return mDevice; }
  const PhysicalDevice *getPhysicalDevice() const { return mPhysicalDevice; }
  const QueueFamilyIndices &getQueueFamilyIndices() const {
    return mQueueFamilyIndices;
  }
  const VkQueue &getGraphicsQueue() const { return mGraphicsQueue; }

private:
  bool isExtensionSupported(const std::string &extension);

  const PhysicalDevice                *mPhysicalDevice;
  VkPhysicalDeviceProperties           mPhysicalDeviceProperties;
  VkPhysicalDeviceFeatures             mPhysicalDeviceFeatures;
  VkPhysicalDeviceMemoryProperties     mPhysicalDeviceMemoryProperties;
  std::vector<VkQueueFamilyProperties> mQueueFamilyProperties;
  std::vector<std::string>             mSupportedExtensions;
  QueueFamilyIndices                   mQueueFamilyIndices;
  VkDevice                             mDevice;
  VkQueue                              mGraphicsQueue;
};
