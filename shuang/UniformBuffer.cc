#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(const std::shared_ptr<Device> &device,
                             VkMemoryPropertyFlags properties, VkDeviceSize size, void *data)
    : Buffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, properties, size, data) {}
