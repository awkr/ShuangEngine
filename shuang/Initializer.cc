#include "Initializer.h"

VkDescriptorBufferInfo createDescriptorBufferInfo(VkBuffer buffer, VkDeviceSize range,
                                                  VkDeviceSize offset) {
  VkDescriptorBufferInfo descriptorBufferInfo{};
  descriptorBufferInfo.buffer = buffer;
  descriptorBufferInfo.range  = range;
  descriptorBufferInfo.offset = offset;
  return descriptorBufferInfo;
}
