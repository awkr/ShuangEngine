#pragma once

#include <vulkan/vulkan.hpp>

VkDescriptorBufferInfo createDescriptorBufferInfo(VkBuffer buffer, VkDeviceSize range,
                                                  VkDeviceSize offset = 0);
