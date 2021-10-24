#include "Device.h"
#include "Logger.h"
#include "Macros.h"
#include "Surface.h"

#include <vulkan/vulkan_beta.h>

Device::Device(const PhysicalDevice *physicalDevice, const Surface *surface)
    : mPhysicalDevice{physicalDevice} {
  // Store properties, features and memory properties of the
  // physical device
  vkGetPhysicalDeviceProperties(physicalDevice->getHandle(),
                                &mPhysicalDeviceProperties);
  vkGetPhysicalDeviceFeatures(physicalDevice->getHandle(),
                              &mPhysicalDeviceFeatures);
  vkGetPhysicalDeviceMemoryProperties(physicalDevice->getHandle(),
                                      &mPhysicalDeviceMemoryProperties);

  // Queue family properties, used for setting up requested queues upon device
  // creation
  uint32_t queueFamilyCount;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice->getHandle(),
                                           &queueFamilyCount, nullptr);
  mQueueFamilyProperties.resize(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice->getHandle(),
                                           &queueFamilyCount,
                                           mQueueFamilyProperties.data());

  // Get list of supported extensions
  uint32_t extensionCount = 0;
  vkEnumerateDeviceExtensionProperties(physicalDevice->getHandle(), nullptr,
                                       &extensionCount, nullptr);
  if (extensionCount > 0) {
    std::vector<VkExtensionProperties> extensions(extensionCount);
    ASSERT(vkEnumerateDeviceExtensionProperties(physicalDevice->getHandle(),
                                                nullptr, &extensionCount,
                                                extensions.data()));
    //    logInfo("Supported device extensions: {}", extensionCount);
    for (const auto &extension : extensions) {
      //      logInfo("  {}", extension.extensionName);
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
    if (vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice->getHandle(), i,
                                             surface->getHandle(),
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
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
  };

  // Check extensions supportability
  for (const auto &extension : deviceExtensions) {
    if (!isExtensionSupported(extension)) {
      throw std::runtime_error(
          fmt::format("Device extension not supported: {}", extension));
    }
  }

  VkDeviceCreateInfo deviceCreateInfo = {};
  deviceCreateInfo.sType              = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.queueCreateInfoCount =
      static_cast<uint32_t>(queueCreateInfos.size());
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
  deviceCreateInfo.enabledExtensionCount =
      static_cast<uint32_t>(deviceExtensions.size());
  deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

  ASSERT(vkCreateDevice(physicalDevice->getHandle(), &deviceCreateInfo, nullptr,
                        &mDevice));

  vkGetDeviceQueue(mDevice, mQueueFamilyIndices.graphics, 0, &mGraphicsQueue);
}

Device::~Device() {
  logInfo(__func__);
  vkDestroyDevice(mDevice, nullptr);
}

bool Device::isExtensionSupported(const std::string &extension) {
  return std::find(mSupportedExtensions.begin(), mSupportedExtensions.end(),
                   extension) != mSupportedExtensions.end();
}
