#pragma once

#include "Buffer.h"

class IndexBuffer : public Buffer {
public:
  IndexBuffer(const std::shared_ptr<Device> &device, VkBufferUsageFlags usage,
              VkMemoryPropertyFlags properties, uint32_t indexCount, VkDeviceSize size,
              void *data = nullptr);

  [[nodiscard]] uint32_t getIndexCount() const { return mIndexCount; }

private:
  uint32_t mIndexCount;
};
