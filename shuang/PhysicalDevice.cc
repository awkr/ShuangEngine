#include "PhysicalDevice.h"
#include "Instance.h"
#include "Logger.h"
#include "Macros.h"

PhysicalDevice::PhysicalDevice(const std::shared_ptr<Instance> &instance) {
  uint32_t gpuCount;
  ASSERT(vkEnumeratePhysicalDevices(instance->getHandle(), &gpuCount, nullptr));
  std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
  ASSERT(vkEnumeratePhysicalDevices(instance->getHandle(), &gpuCount,
                                    physicalDevices.data()));

  // GPU selection: defaults to the first device
  uint32_t selectedDevice = 0;
  mHandle                 = physicalDevices[selectedDevice];

  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(mHandle, &properties);

  log_info("Pick GPU: {}", properties.deviceName);
}

PhysicalDevice::~PhysicalDevice() { log_func; }
