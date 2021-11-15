#include "Validation.h"
#include "Log.h"

namespace validation {

PFN_vkCreateDebugUtilsMessengerEXT  vkCreateMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyMessengerEXT;
VkDebugUtilsMessengerEXT            messenger;

void setup(const VkInstance instance, VkDebugReportFlagsEXT flags,
           VkDebugReportCallbackEXT callback) {
  vkCreateMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
  vkDestroyMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

  VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{};
  debugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  debugUtilsMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  debugUtilsMessengerCreateInfo.messageType =
      VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
  debugUtilsMessengerCreateInfo.pfnUserCallback = messengerCallback;
  auto result = vkCreateMessengerEXT(instance, &debugUtilsMessengerCreateInfo, nullptr, &messenger);
  assert(result == VK_SUCCESS);
}

void cleanup(const VkInstance instance) { vkDestroyMessengerEXT(instance, messenger, nullptr); }

VKAPI_ATTR VkBool32 VKAPI_CALL
messengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT             messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
  std::stringstream message;
  message << "[" << pCallbackData->pMessageIdName << "] : " << pCallbackData->pMessage;

  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
    log_debug(message.str());
  } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
    log_info(message.str());
  } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    log_warn(message.str());
  } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    log_error(message.str());
  }

  // The return value of this callback controls whether the Vulkan call that
  // caused the validation message will be aborted or not We return VK_FALSE as
  // we DON'T want Vulkan calls that cause a validation message to abort If you
  // instead want to have calls abort, pass in VK_TRUE and the function will
  // return VK_ERROR_VALIDATION_FAILED_EXT
  return VK_FALSE;
}

} // namespace validation
