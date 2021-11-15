#include "PhysicalDevice.h"
#include "Instance.h"
#include "Log.h"
#include "Macros.h"

PhysicalDevice::PhysicalDevice(const std::shared_ptr<Instance> &instance) {
  uint32_t gpuCount;
  vkOK(vkEnumeratePhysicalDevices(instance->getHandle(), &gpuCount, nullptr));
  std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
  vkOK(vkEnumeratePhysicalDevices(instance->getHandle(), &gpuCount, physicalDevices.data()));

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

VkFormat PhysicalDevice::getSuitableDepthFormat(bool                         depthOnly,
                                                const std::vector<VkFormat> &formats) const {
  for (const auto &format : formats) {
    if (depthOnly && !isDepthOnlyFormat(format)) {
      continue;
    }

    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(mHandle, format, &properties);

    // Format must support depth stencil attachment for optimal tiling
    if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
      return format;
    }
  }
  return VK_FORMAT_UNDEFINED;
}

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

bool PhysicalDevice::isDepthOnlyFormat(VkFormat format) {
  return format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT;
}
