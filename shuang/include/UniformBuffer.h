#pragma once

#include "Buffer.h"

class UniformBuffer : public Buffer {
public:
  UniformBuffer(const std::shared_ptr<Device> &device, VkMemoryPropertyFlags properties,
                VkDeviceSize size, void *data = nullptr);
};
