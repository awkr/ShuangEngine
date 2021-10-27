#include "IndexBuffer.h"

IndexBuffer::IndexBuffer(const std::shared_ptr<Device> &device, VkBufferUsageFlags usage,
                         VkMemoryPropertyFlags properties, uint32_t indexCount, VkDeviceSize size,
                         void *data)
    : Buffer(device, usage | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, properties, size, data),
      mIndexCount{indexCount} {}
