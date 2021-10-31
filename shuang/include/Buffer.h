#pragma once

#include <vulkan/vulkan.hpp>

class Device;

class Buffer {
public:
  Buffer(const std::shared_ptr<Device> &device, VkBufferUsageFlags usage,
         VkMemoryPropertyFlags properties, VkDeviceSize size, void *data = nullptr);
  virtual ~Buffer();

  [[nodiscard]] const VkBuffer     &getHandle() const { return mHandle; }
  [[nodiscard]] const VkDeviceSize &getSize() const { return mSize; }
  /**
   * @brief Copies from a buffer
   * @param src The buffer to copy from
   * @param queue The queue to submit the copy command to
   * @param region The amount to copy, if null copies the entire buffer
   */
  void copy(const std::shared_ptr<Buffer> &src, VkQueue queue, VkBufferCopy *region = nullptr);
  void copy(void *data, size_t size);

private:
  const std::shared_ptr<Device> &mDevice;
  VkBuffer                       mHandle = VK_NULL_HANDLE;
  VkDeviceMemory                 mMemory = VK_NULL_HANDLE;
  VkDeviceSize                   mSize;
};
