#pragma once

class Device;
class Surface;

#include <vulkan/vulkan.hpp>

class Swapchain {
public:
  struct FrameClip {
    VkCommandPool   primaryCommandPool   = VK_NULL_HANDLE;
    VkCommandBuffer primaryCommandBuffer = VK_NULL_HANDLE;
    VkFence         queueSubmittedFence  = VK_NULL_HANDLE;
    VkSemaphore     acquiredSemaphore    = VK_NULL_HANDLE;
    // Signaled when the queue finished executing the commands in the buffer
    VkSemaphore releasedSemaphore = VK_NULL_HANDLE;
  };

  Swapchain(const Device *device, const Surface *surface);
  ~Swapchain();

  const VkSwapchainKHR &getHandle() const { return mHandle; }
  const VkFormat       &getImageFormat() const { return mImageFormat; }
  const std::vector<VkImageView> &getImageViews() const { return mImageViews; }
  const VkExtent2D       &getImageExtent() const { return mImageExtent; }
  std::vector<FrameClip> &getFrameClips() { return mFrameClips; }

  VkResult acquireNextImage(uint32_t &imageIndex);

private:
  void initFrameClip(FrameClip &frameClip);

  const Device            *mDevice;
  VkExtent2D               mImageExtent;
  VkFormat                 mImageFormat;
  VkSwapchainKHR           mHandle;
  uint32_t                 mImageCount;
  std::vector<VkImage>     mImages;
  std::vector<VkImageView> mImageViews;
  std::vector<FrameClip>   mFrameClips;
  std::vector<VkSemaphore> mSemaphorePool;
};
