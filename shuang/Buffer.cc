#include "Buffer.h"
#include "Device.h"
#include "Logger.h"
#include "Macros.h"

Buffer::Buffer(const std::shared_ptr<Device> &device, VkBufferUsageFlags usage,
               VkMemoryPropertyFlags properties, VkDeviceSize size, void *data)
    : mDevice{device}, mSize{size} {
  // Create the buffer handle
  VkBufferCreateInfo bufferCreateInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  bufferCreateInfo.usage       = usage;
  bufferCreateInfo.size        = size;
  bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  vkOK(vkCreateBuffer(device->getHandle(), &bufferCreateInfo, nullptr, &mHandle));

  // Create the memory backing up the buffer handle
  VkMemoryRequirements memoryRequirements;
  vkGetBufferMemoryRequirements(device->getHandle(), mHandle, &memoryRequirements);

  VkMemoryAllocateInfo memoryAllocateInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  memoryAllocateInfo.allocationSize = memoryRequirements.size;
  // Find a memory type index that fits the properties of the buffer
  memoryAllocateInfo.memoryTypeIndex =
      device->getPhysicalDevice()->getMemoryType(memoryRequirements.memoryTypeBits, properties);
  vkOK(vkAllocateMemory(device->getHandle(), &memoryAllocateInfo, nullptr, &mMemory));

  // If a pointer to the buffer data has been passed, map the buffer and copy data
  if (data != nullptr) {
    void *mapped;
    vkOK(vkMapMemory(device->getHandle(), mMemory, 0, size, 0, &mapped));
    memcpy(mapped, data, static_cast<size_t>(size));
    // If host coherency hasn't been requested, do a manual flush to make writes visible
    if ((properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
      VkMappedMemoryRange mappedRange{VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};
      mappedRange.memory = mMemory;
      mappedRange.offset = 0;
      mappedRange.size   = size;
      vkFlushMappedMemoryRanges(device->getHandle(), 1, &mappedRange);
    }
    vkUnmapMemory(device->getHandle(), mMemory);
  }

  // Attach the memory to the buffer object
  vkOK(vkBindBufferMemory(device->getHandle(), mHandle, mMemory, 0));
}

Buffer::~Buffer() {
  log_func;
  vkFreeMemory(mDevice->getHandle(), mMemory, nullptr);
  vkDestroyBuffer(mDevice->getHandle(), mHandle, nullptr);
}

void Buffer::copy(const std::shared_ptr<Buffer> &src, VkQueue queue, VkBufferCopy *region) {
  VkBufferCopy bufferCopy{};
  if (!region) {
    bufferCopy.size = src->getSize();
  } else {
    bufferCopy = *region;
  }

  auto copyCommand = mDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
  vkCmdCopyBuffer(copyCommand, src->getHandle(), mHandle, 1, &bufferCopy);
  mDevice->flushCommandBuffer(copyCommand, queue);
}

void Buffer::copy(void *data, size_t size) {
  void *mapped;
  vkOK(vkMapMemory(mDevice->getHandle(), mMemory, 0, size, 0, &mapped));
  memcpy(mapped, data, size);
  vkUnmapMemory(mDevice->getHandle(), mMemory);
}
