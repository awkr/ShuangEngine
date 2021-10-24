#include "PhysicalDevice.h"
#include "Instance.h"
#include "Macros.h"

PhysicalDevice::PhysicalDevice(const Instance *instance) {
  uint32_t gpuCount;
  ASSERT(vkEnumeratePhysicalDevices(instance->getHandle(), &gpuCount, nullptr));
  std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
  ASSERT(vkEnumeratePhysicalDevices(instance->getHandle(), &gpuCount,
                                    physicalDevices.data()));

  // GPU selection: defaults to the first device
  uint32_t selectedDevice = 0;
  mPhysicalDevice         = physicalDevices[selectedDevice];
}
