#pragma once

#include <vulkan/vulkan.hpp>

#include "Buffer.h"
#include "CommandPool.h"
#include "PhysicalDevice.h"

class Surface;

class Device {
public:
  struct QueueFamilyIndices {
    uint32_t graphics;
    uint32_t compute;
    uint32_t transfer;
  };

  Device(const std::shared_ptr<PhysicalDevice> &physicalDevice,
         const std::shared_ptr<Surface>        &surface);
  ~Device();

  const VkDevice                        &getHandle() const { return mHandle; }
  const std::shared_ptr<PhysicalDevice> &getPhysicalDevice() const { return mPhysicalDevice; }
  const QueueFamilyIndices &getQueueFamilyIndices() const { return mQueueFamilyIndices; }
  const VkQueue            &getGraphicsQueue() const { return mGraphicsQueue; }
  VkResult                  waitIdle() const { return vkDeviceWaitIdle(mHandle); }

  /**
   * @brief Requests a command buffer from the device's command pool
   * @param level The command buffer level
   * @param begin Whether the command buffer should be implicitly started before it's returned
   * @returns A valid VkCommandBuffer
   */
  VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin = false);

  /**
   * @brief Submits and frees up a given command buffer
   * @param commandBuffer The command buffer
   * @param queue The queue to submit the work to
   * @param free Whether the command buffer should be implicitly freed up
   * @param signalSemaphore An optional semaphore to signal when the commands have been executed
   */
  void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true,
                          VkSemaphore signalSemaphore = VK_NULL_HANDLE);

private:
  bool supportsExtension(const std::string &extension);

  const std::shared_ptr<PhysicalDevice> &mPhysicalDevice = nullptr;
  std::vector<VkQueueFamilyProperties>   mQueueFamilyProperties;
  std::vector<std::string>               mSupportedExtensions;
  QueueFamilyIndices                     mQueueFamilyIndices;
  VkDevice                               mHandle        = VK_NULL_HANDLE;
  VkQueue                                mGraphicsQueue = VK_NULL_HANDLE;
  // A command pool associated to the primary queue
  std::unique_ptr<CommandPool> mCommandPool;
};
