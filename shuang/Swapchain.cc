#include "Swapchain.h"
#include "Device.h"
#include "Logger.h"
#include "Macros.h"
#include "RenderPass.h"
#include "Surface.h"

Swapchain::Swapchain(const std::shared_ptr<Device>  &device,
                     const std::shared_ptr<Surface> &surface)
    : mDevice{device} {
  VkSurfaceCapabilitiesKHR capabilities;
  ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      device->getPhysicalDevice()->getHandle(), surface->getHandle(),
      &capabilities));

  uint32_t surfaceFormatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device->getPhysicalDevice()->getHandle(),
                                       surface->getHandle(),
                                       &surfaceFormatCount, nullptr);
  if (surfaceFormatCount == 0) {
    throw std::runtime_error("Surface has no formats.");
  }
  std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(
      device->getPhysicalDevice()->getHandle(), surface->getHandle(),
      &surfaceFormatCount, surfaceFormats.data());

  VkSurfaceFormatKHR surfaceFormat;
  if (surfaceFormatCount == 1 &&
      surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
    // There is no preferred format, so pick a default one
    surfaceFormat        = surfaceFormats[0];
    surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
  } else {
    surfaceFormat.format = VK_FORMAT_UNDEFINED;
    for (const auto &candidate : surfaceFormats) {
      switch (candidate.format) {
      case VK_FORMAT_R8G8B8A8_UNORM:
      case VK_FORMAT_B8G8R8A8_UNORM:
      case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        surfaceFormat = candidate;
        break;
      default:
        break;
      }

      if (surfaceFormat.format != VK_FORMAT_UNDEFINED) {
        break;
      }
    }

    if (surfaceFormat.format == VK_FORMAT_UNDEFINED) {
      surfaceFormat = surfaceFormats[0];
    }
  }

  mImageFormat = surfaceFormat.format;

  mImageExtent = capabilities.currentExtent;
  if (capabilities.currentExtent.width == 0xFFFFFFFF) {
    mImageExtent.width  = surface->getExtent().width;
    mImageExtent.height = surface->getExtent().height;
  }

  // FIFO must be supported by all implementations.
  VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

  // Determine the number of VkImage's to use in the swapchain.
  // Ideally, we desire to own 1 image at a time, the rest of the images can
  // either be rendered to and/or being queued up for display.
  uint32_t desiredImages = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 &&
      desiredImages > capabilities.maxImageCount) {
    // Application must settle for fewer images than desired.
    desiredImages = capabilities.maxImageCount;
  }

  // Figure out a suitable surface transform.
  VkSurfaceTransformFlagBitsKHR preTransform;
  if (capabilities.supportedTransforms &
      VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
    preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  } else {
    preTransform = capabilities.currentTransform;
  }

  // Find a supported composite type.
  VkCompositeAlphaFlagBitsKHR composite = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  if (capabilities.supportedCompositeAlpha &
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
    composite = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  } else if (capabilities.supportedCompositeAlpha &
             VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) {
    composite = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
  } else if (capabilities.supportedCompositeAlpha &
             VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) {
    composite = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
  } else if (capabilities.supportedCompositeAlpha &
             VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) {
    composite = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
  }

  VkSwapchainCreateInfoKHR swapchainCreateInfo{
      VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
  swapchainCreateInfo.surface            = surface->getHandle();
  swapchainCreateInfo.minImageCount      = desiredImages;
  swapchainCreateInfo.imageFormat        = mImageFormat;
  swapchainCreateInfo.imageColorSpace    = surfaceFormat.colorSpace;
  swapchainCreateInfo.imageExtent.width  = mImageExtent.width;
  swapchainCreateInfo.imageExtent.height = mImageExtent.height;
  swapchainCreateInfo.imageArrayLayers   = 1;
  swapchainCreateInfo.imageUsage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchainCreateInfo.imageSharingMode   = VK_SHARING_MODE_EXCLUSIVE;
  swapchainCreateInfo.preTransform       = preTransform;
  swapchainCreateInfo.compositeAlpha     = composite;
  swapchainCreateInfo.presentMode        = presentMode;
  swapchainCreateInfo.clipped            = true;

  ASSERT(vkCreateSwapchainKHR(mDevice->getHandle(), &swapchainCreateInfo,
                              nullptr, &mHandle));

  ASSERT(vkGetSwapchainImagesKHR(mDevice->getHandle(), mHandle, &mImageCount,
                                 nullptr));
  mImages.resize(mImageCount);
  ASSERT(vkGetSwapchainImagesKHR(mDevice->getHandle(), mHandle, &mImageCount,
                                 mImages.data()));

  for (size_t i = 0; i < mImageCount; ++i) {
    // Create an image view which we can render into.
    VkImageViewCreateInfo imageViewCreateInfo{
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    imageViewCreateInfo.viewType                    = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format                      = surfaceFormat.format;
    imageViewCreateInfo.image                       = mImages[i];
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCreateInfo.components.r                = VK_COMPONENT_SWIZZLE_R;
    imageViewCreateInfo.components.g                = VK_COMPONENT_SWIZZLE_G;
    imageViewCreateInfo.components.b                = VK_COMPONENT_SWIZZLE_B;
    imageViewCreateInfo.components.a                = VK_COMPONENT_SWIZZLE_A;

    VkImageView imageView;
    ASSERT(vkCreateImageView(mDevice->getHandle(), &imageViewCreateInfo,
                             nullptr, &imageView));

    mImageViews.push_back(imageView);
  }

  // Initialize per-frame resources.
  // Every swapchain image has its own command pool and fence manager.
  // This makes it very easy to keep track of when we can reset command buffers
  // and such.
  mFrameClips.clear();
  mFrameClips.resize(mImageCount);

  for (size_t i = 0; i < mImageCount; ++i) {
    initFrameClip(mFrameClips[i]);
  }
}

Swapchain::~Swapchain() {
  log_func;

  for (auto &semaphore : mSemaphorePool) {
    vkDestroySemaphore(mDevice->getHandle(), semaphore, nullptr);
  }

  for (auto &frameClip : mFrameClips) {
    vkDestroySemaphore(mDevice->getHandle(), frameClip.releasedSemaphore,
                       nullptr);
    vkDestroySemaphore(mDevice->getHandle(), frameClip.acquiredSemaphore,
                       nullptr);
    vkFreeCommandBuffers(mDevice->getHandle(), frameClip.primaryCommandPool, 1,
                         &frameClip.primaryCommandBuffer);
    vkDestroyCommandPool(mDevice->getHandle(), frameClip.primaryCommandPool,
                         nullptr);
    vkDestroyFence(mDevice->getHandle(), frameClip.queueSubmittedFence,
                   nullptr);
  }

  cleanupFramebuffers();

  for (size_t i = 0; i < mImageCount; i++) {
    vkDestroyImageView(mDevice->getHandle(), mImageViews[i], nullptr);
  }

  vkDestroySwapchainKHR(mDevice->getHandle(), mHandle, nullptr);
}

void Swapchain::initFrameClip(FrameClip &frameClip) {
  VkFenceCreateInfo fenceCreateInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
  fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  ASSERT(vkCreateFence(mDevice->getHandle(), &fenceCreateInfo, nullptr,
                       &frameClip.queueSubmittedFence));

  VkCommandPoolCreateInfo commandPoolCreateInfo{
      VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
  commandPoolCreateInfo.queueFamilyIndex =
      mDevice->getQueueFamilyIndices().graphics;
  ASSERT(vkCreateCommandPool(mDevice->getHandle(), &commandPoolCreateInfo,
                             nullptr, &frameClip.primaryCommandPool));

  VkCommandBufferAllocateInfo commandBufferAllocateInfo{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  commandBufferAllocateInfo.commandPool = frameClip.primaryCommandPool;
  commandBufferAllocateInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  commandBufferAllocateInfo.commandBufferCount = 1;
  ASSERT(vkAllocateCommandBuffers(mDevice->getHandle(),
                                  &commandBufferAllocateInfo,
                                  &frameClip.primaryCommandBuffer));
}

void Swapchain::createFramebuffers(
    const std::shared_ptr<RenderPass> &renderPass) {
  for (const auto &imageView : mImageViews) {
    // Build the framebuffer.
    VkFramebufferCreateInfo framebufferCreateInfo{
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    framebufferCreateInfo.renderPass      = renderPass->getHandle();
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.pAttachments    = &imageView;
    framebufferCreateInfo.width           = mImageExtent.width;
    framebufferCreateInfo.height          = mImageExtent.height;
    framebufferCreateInfo.layers          = 1;

    VkFramebuffer framebuffer;
    ASSERT(vkCreateFramebuffer(mDevice->getHandle(), &framebufferCreateInfo,
                               nullptr, &framebuffer));
    mFramebuffers.push_back(framebuffer);
  }
}

void Swapchain::cleanupFramebuffers() {
  vkQueueWaitIdle(mDevice->getGraphicsQueue());

  for (auto &framebuffer : mFramebuffers) {
    vkDestroyFramebuffer(mDevice->getHandle(), framebuffer, nullptr);
  }

  mFramebuffers.clear();
}

VkResult Swapchain::acquireNextImage(uint32_t &imageIndex) {
  VkSemaphore semaphore;
  if (mSemaphorePool.empty()) {
    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    ASSERT(vkCreateSemaphore(mDevice->getHandle(), &semaphoreCreateInfo,
                             nullptr, &semaphore));
  } else {
    semaphore = mSemaphorePool.back();
    mSemaphorePool.pop_back();
  }

  auto result = vkAcquireNextImageKHR(mDevice->getHandle(), mHandle, UINT64_MAX,
                                      semaphore, VK_NULL_HANDLE, &imageIndex);
  if (result != VK_SUCCESS) {
    mSemaphorePool.push_back(semaphore);
    return result;
  }

  auto &frameClip = mFrameClips[imageIndex];

  // If we have outstanding fence for this swapchain imageIndex, wait for it
  // to complete first. After begin frame returns, it is safe to reuse or delete
  // resources which were used previously.
  //
  // We wait for fence which completes N frames earlier, so we do not stall,
  // waiting for all GPU work to complete before this returns.
  // Normally, this doesn't really block at all, since we're waiting for old
  // frames to have been completed, but just in case.
  if (frameClip.queueSubmittedFence != VK_NULL_HANDLE) {
    vkWaitForFences(mDevice->getHandle(), 1, &frameClip.queueSubmittedFence,
                    true, UINT64_MAX);
    vkResetFences(mDevice->getHandle(), 1, &frameClip.queueSubmittedFence);
  }

  if (frameClip.primaryCommandPool != VK_NULL_HANDLE) {
    vkResetCommandPool(mDevice->getHandle(), frameClip.primaryCommandPool, 0);
  }

  // Recycle the old semaphore back into the semaphore pool.
  if (frameClip.acquiredSemaphore != VK_NULL_HANDLE) {
    mSemaphorePool.push_back(frameClip.acquiredSemaphore);
  }

  frameClip.acquiredSemaphore = semaphore;

  return VK_SUCCESS;
}
