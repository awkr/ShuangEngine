#include "VertexBuffer.h"

VertexBuffer::VertexBuffer(const std::shared_ptr<Device> &device, VkBufferUsageFlags usage,
                           VkMemoryPropertyFlags properties, int count, int stride, void *data)
    : Buffer(device, usage | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, properties, count * stride, data),
      mCount{count} {}
