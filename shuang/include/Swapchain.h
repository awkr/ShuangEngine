#pragma once

class Device;
class Surface;
class RenderPass;

#include <vulkan/vulkan.hpp>

class Swapchain {
public:
  struct Frame {
    VkCommandPool   primaryCommandPool   = VK_NULL_HANDLE;
    VkCommandBuffer primaryCommandBuffer = VK_NULL_HANDLE;
    VkFence         queueSubmittedFence  = VK_NULL_HANDLE;
    VkSemaphore     acquiredSemaphore    = VK_NULL_HANDLE;
    // Signaled when the queue finished executing the commands in the buffer
    VkSemaphore releasedSemaphore = VK_NULL_HANDLE;
  };

  Swapchain(const std::shared_ptr<Device> &device, const std::shared_ptr<Surface> &surface);
  ~Swapchain();

  const VkSwapchainKHR           &getHandle() const { return mHandle; }
  const VkFormat                 &getImageFormat() const { return mImageFormat; }
  const std::vector<VkImageView> &getImageViews() const { return mImageViews; }
  const VkExtent2D               &getImageExtent() const { return mImageExtent; }
  std::vector<Frame>             &getFrames() { return mFrames; }
  std::vector<VkFramebuffer>     &getFramebuffers() { return mFramebuffers; }

  void     createFramebuffers(const std::shared_ptr<RenderPass> &renderPass);
  VkResult acquireNextImage(uint32_t &imageIndex);

private:
  void initFrame(Frame &frame);
  void cleanupFramebuffers();

  const std::shared_ptr<Device> &mDevice = nullptr;
  VkExtent2D                     mImageExtent;
  VkFormat                       mImageFormat;
  VkSwapchainKHR                 mHandle;
  uint32_t                       mImageCount;
  std::vector<VkImage>           mImages;
  std::vector<VkImageView>       mImageViews;
  std::vector<Frame>             mFrames;
  std::vector<VkSemaphore>       mSemaphorePool;
  // Framebuffers for each image view
  std::vector<VkFramebuffer> mFramebuffers;
};
