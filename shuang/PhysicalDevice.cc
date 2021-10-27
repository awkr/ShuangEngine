#include "PhysicalDevice.h"
#include "Instance.h"
#include "Logger.h"
#include "Macros.h"

PhysicalDevice::PhysicalDevice(const std::shared_ptr<Instance> &instance) {
  uint32_t gpuCount;
  vkAssert(vkEnumeratePhysicalDevices(instance->getHandle(), &gpuCount, nullptr));
  std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
  vkAssert(vkEnumeratePhysicalDevices(instance->getHandle(), &gpuCount, physicalDevices.data()));

  // GPU selection: defaults to the first device
  uint32_t selectedDevice = 0;
  mHandle                 = physicalDevices[selectedDevice];

  // Store properties, features and memory properties of the selected GPU
  vkGetPhysicalDeviceProperties(mHandle, &mProperties);
  vkGetPhysicalDeviceFeatures(mHandle, &mFeatures);
  vkGetPhysicalDeviceMemoryProperties(mHandle, &mMemoryProperties);

  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(mHandle, &properties);

  log_info("Pick GPU: {}", properties.deviceName);
}

PhysicalDevice::~PhysicalDevice() { log_func; }

uint32_t PhysicalDevice::getMemoryType(uint32_t bits, VkMemoryPropertyFlags properties,
                                       VkBool32 *found) {
  for (uint32_t i = 0; i < mMemoryProperties.memoryTypeCount; ++i) {
    if ((bits & 1) == 1) {
      if ((mMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
        if (found) {
          *found = true;
        }
        return i;
      }
    }
    bits >>= 1;
  }

  if (found) {
    *found = false;
    return 0;
  }

  throw std::runtime_error("Could not find a matching memory type");
}
