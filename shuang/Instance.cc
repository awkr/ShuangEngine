#include "Instance.h"
#include "Logger.h"
#include "Macros.h"
#include "Validation.h"

#include <vulkan/vulkan_metal.h>

Instance::Instance(const char *applicationName, bool enableValidation)
    : mEnableValidation{enableValidation} {
  // Get extensions supported by the instance
  uint32_t extensionCount;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  if (extensionCount > 0) {
    std::vector<VkExtensionProperties> extensions(extensionCount);
    if (vkEnumerateInstanceExtensionProperties(
            nullptr, &extensionCount, extensions.data()) == VK_SUCCESS) {
      //      logInfo("Supported instance extensions: {}", extensionCount);
      for (const auto &extension : extensions) {
        //        logInfo("  {}", extension.extensionName);
        mSupportedInstanceExtensions.emplace_back(extension.extensionName);
      }
    }
  }

  std::vector<const char *> instanceExtensions = {
      VK_KHR_SURFACE_EXTENSION_NAME,
      VK_EXT_METAL_SURFACE_EXTENSION_NAME,
  };
  if (mEnableValidation) {
    instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  for (const auto &extension : mRequestedInstanceExtensions) {
    if (std::find(mSupportedInstanceExtensions.begin(),
                  mSupportedInstanceExtensions.end(),
                  extension) == mSupportedInstanceExtensions.end()) {
      throw std::runtime_error(fmt::format(
          "Requested instance extension not present: {}", extension));
    }
    instanceExtensions.push_back(extension);
  }

  VkApplicationInfo applicationInfo{};
  applicationInfo.sType            = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  applicationInfo.pApplicationName = applicationName;
  applicationInfo.pEngineName      = "Shuang";
  applicationInfo.apiVersion       = VK_API_VERSION_1_2;

  VkInstanceCreateInfo instanceCreateInfo{};
  instanceCreateInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCreateInfo.pNext            = nullptr;
  instanceCreateInfo.pApplicationInfo = &applicationInfo;

  if (!instanceExtensions.empty()) {
    instanceCreateInfo.enabledExtensionCount =
        (uint32_t)instanceExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
  }

  const char *validationLayerName = "VK_LAYER_KHRONOS_validation";
  if (mEnableValidation) {
    uint32_t instanceLayerCount;
    vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
    std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
    vkEnumerateInstanceLayerProperties(&instanceLayerCount,
                                       instanceLayerProperties.data());
    bool isPresent = false;
    for (const auto &layer : instanceLayerProperties) {
      if (strcmp(layer.layerName, validationLayerName) == 0) {
        isPresent = true;
        break;
      }
    }
    if (isPresent) {
      instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
      instanceCreateInfo.enabledLayerCount   = 1;
    } else {
      logError("Layer VK_LAYER_KHRONOS_validation not present");
    }
  }

  ASSERT(vkCreateInstance(&instanceCreateInfo, nullptr, &mHandle));

  if (enableValidation) {
    VkDebugReportFlagsEXT reportFlags = VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                        VK_DEBUG_REPORT_ERROR_BIT_EXT |
                                        VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT;
    validation::setup(mHandle, reportFlags, VK_NULL_HANDLE);
  }
}

Instance::~Instance() {
  logInfo(__func__);

  if (mEnableValidation) {
    validation::cleanup(mHandle);
  }

  vkDestroyInstance(mHandle, nullptr);
}
