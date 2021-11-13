#pragma once

#include "Buffer.h"

class VertexBuffer : public Buffer {
public:
  VertexBuffer(const std::shared_ptr<Device> &device, VkBufferUsageFlags usage,
               VkMemoryPropertyFlags properties, int count, int stride, void *data = nullptr);

  // Return vertex count
  [[nodiscard]] int getCount() const { return mCount; }

private:
  int mCount;
};
