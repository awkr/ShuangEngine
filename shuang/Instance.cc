#include "Instance.h"
#include "Logger.h"
#include "Macros.h"
#include "Validation.h"

#include <vulkan/vulkan_metal.h>

Instance::Instance(const char *applicationName, const std::vector<const char *> &requiredExtensions,
                   bool enableValidation)
    : mEnableValidation{enableValidation} {
  // Get extensions supported by the instance
  uint32_t extensionCount;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  if (extensionCount > 0) {
    std::vector<VkExtensionProperties> extensions(extensionCount);
    if (vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data()) ==
        VK_SUCCESS) {
      //      log_info("Supported instance extensions: {}", extensionCount);
      for (const auto &extension : extensions) {
        //        log_info("  {}", extension.extensionName);
        mSupportedExtensions.emplace_back(extension.extensionName);
      }
    }
  }

  std::vector<const char *> extensions = {
      VK_KHR_SURFACE_EXTENSION_NAME,
      VK_EXT_METAL_SURFACE_EXTENSION_NAME,
  };
  if (enableValidation) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  for (const auto &extension : requiredExtensions) {
    if (!isExtensionSupport(extension)) {
      throw std::runtime_error(
          fmt::format("Requested instance extension not present: {}", extension));
    }
    extensions.emplace_back(extension);
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

  if (!extensions.empty()) {
    instanceCreateInfo.enabledExtensionCount   = (uint32_t)extensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
  }

  if (enableValidation) {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    mSupportedLayers.resize(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, mSupportedLayers.data());

    const char *validationLayer = "VK_LAYER_KHRONOS_validation";
    if (isLayerSupport(validationLayer)) {
      instanceCreateInfo.ppEnabledLayerNames = &validationLayer;
      instanceCreateInfo.enabledLayerCount   = 1;
    } else {
      log_error("Layer not present: {}", validationLayer);
    }
  }

  vkAssert(vkCreateInstance(&instanceCreateInfo, nullptr, &mHandle));

  if (enableValidation) {
    VkDebugReportFlagsEXT reportFlags = VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                        VK_DEBUG_REPORT_ERROR_BIT_EXT |
                                        VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT;
    validation::setup(mHandle, reportFlags, VK_NULL_HANDLE);
  }
}

Instance::~Instance() {
  log_func;

  if (mEnableValidation) {
    validation::cleanup(mHandle);
  }

  vkDestroyInstance(mHandle, nullptr);
}

bool Instance::isExtensionSupport(const char *extension) {
  return std::find(mSupportedExtensions.begin(), mSupportedExtensions.end(), extension) !=
         mSupportedExtensions.end();
}

bool Instance::isLayerSupport(const char *name) {
  for (const auto &layer : mSupportedLayers) {
    if (equals(name, layer.layerName)) {
      return true;
    }
  }
  return false;
}

bool Instance::equals(const char *a, const char *b) { return strcmp(a, b) == 0; }
