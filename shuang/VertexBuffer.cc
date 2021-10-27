#include "VertexBuffer.h"

VertexBuffer::VertexBuffer(const std::shared_ptr<Device> &device, VkBufferUsageFlags usage,
                           VkMemoryPropertyFlags properties, VkDeviceSize size, void *data)
    : Buffer(device, usage | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, properties, size, data) {}
