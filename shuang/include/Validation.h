#pragma once

#include <vulkan/vulkan.hpp>

namespace validation {

// Load debug function pointers and set debug callback.
// If callback is nil, default message callback will be used.
void setup(const VkInstance instance, VkDebugReportFlagsEXT flags,
           VkDebugReportCallbackEXT callback);

void cleanup(const VkInstance instance);

VKAPI_ATTR VkBool32 VKAPI_CALL messengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

} // namespace validation
