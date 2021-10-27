#pragma once

#include "Buffer.h"

class VertexBuffer : public Buffer {
public:
  VertexBuffer(const std::shared_ptr<Device> &device, VkBufferUsageFlags usage,
               VkMemoryPropertyFlags properties, VkDeviceSize size, void *data = nullptr);
};
