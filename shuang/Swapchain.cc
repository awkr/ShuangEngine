#include "Swapchain.h"
#include "Device.h"
#include "Logger.h"
#include "Macros.h"
#include "RenderPass.h"
#include "Surface.h"

Swapchain::Swapchain(const std::shared_ptr<Device> &device, const std::shared_ptr<Surface> &surface)
    : mDevice{device} {
  VkSurfaceCapabilitiesKHR capabilities;
  vkOK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->getPhysicalDevice()->getHandle(),
                                                     surface->getHandle(), &capabilities));

  uint32_t surfaceFormatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device->getPhysicalDevice()->getHandle(),
                                       surface->getHandle(), &surfaceFormatCount, nullptr);
  if (surfaceFormatCount == 0) {
    throw std::runtime_error("Surface has no formats.");
  }
  std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(device->getPhysicalDevice()->getHandle(),
                                       surface->getHandle(), &surfaceFormatCount,
                                       surfaceFormats.data());

  VkSurfaceFormatKHR surfaceFormat;
  if (surfaceFormatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
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

  uint32_t presentModesCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device->getPhysicalDevice()->getHandle(),
                                            surface->getHandle(), &presentModesCount, nullptr);
  std::vector<VkPresentModeKHR> presentModes(presentModesCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(device->getPhysicalDevice()->getHandle(),
                                            surface->getHandle(), &presentModesCount,
                                            presentModes.data());

  // FIFO must be supported by all implementations.
  VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

  // Determine the number of VkImage's to use in the swapchain.
  // Ideally, we desire to own 1 image at a time, the rest of the images can
  // either be rendered to and/or being queued up for display.
  // uint32_t desiredImages = capabilities.minImageCount + 1; // TODO FPS will be 120, yah !
  uint32_t desiredImages = 2;
  if (capabilities.maxImageCount > 0 && desiredImages > capabilities.maxImageCount) {
    // Application must settle for fewer images than desired.
    desiredImages = capabilities.maxImageCount;
  }

  // Figure out a suitable surface transform.
  VkSurfaceTransformFlagBitsKHR preTransform;
  if (capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
    preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  } else {
    preTransform = capabilities.currentTransform;
  }

  // Find a supported composite type.
  VkCompositeAlphaFlagBitsKHR composite = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  if (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
    composite = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  } else if (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) {
    composite = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
  } else if (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) {
    composite = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
  } else if (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) {
    composite = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
  }

  VkSwapchainCreateInfoKHR swapchainCreateInfo{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
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

  vkOK(vkCreateSwapchainKHR(mDevice->getHandle(), &swapchainCreateInfo, nullptr, &mHandle));

  vkOK(vkGetSwapchainImagesKHR(mDevice->getHandle(), mHandle, &mImageCount, nullptr));
  mImages.resize(mImageCount);
  vkOK(vkGetSwapchainImagesKHR(mDevice->getHandle(), mHandle, &mImageCount, mImages.data()));

  for (size_t i = 0; i < mImageCount; ++i) {
    // Create an image view which we can render into.
    VkImageViewCreateInfo imageViewCreateInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
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
    vkOK(vkCreateImageView(mDevice->getHandle(), &imageViewCreateInfo, nullptr, &imageView));

    mImageViews.push_back(imageView);
  }

  // Initialize per-frame resources.
  // Every swapchain image has its own command pool and fence manager.
  // This makes it very easy to keep track of when we can reset command buffers
  // and such.
  mFrames.clear();
  mFrames.resize(mImageCount);

  for (size_t i = 0; i < mImageCount; ++i) {
    initFrame(mFrames[i]);
  }

  mDepthFormat = mDevice->getPhysicalDevice()->getSuitableDepthFormat();
}

Swapchain::~Swapchain() {
  log_func;

  for (auto &semaphore : mSemaphorePool) {
    vkDestroySemaphore(mDevice->getHandle(), semaphore, nullptr);
  }

  for (auto &frame : mFrames) {
    vkDestroySemaphore(mDevice->getHandle(), frame.releasedSemaphore, nullptr);
    vkDestroySemaphore(mDevice->getHandle(), frame.acquiredSemaphore, nullptr);
    vkFreeCommandBuffers(mDevice->getHandle(), frame.primaryCommandPool, 1,
                         &frame.primaryCommandBuffer);
    vkDestroyCommandPool(mDevice->getHandle(), frame.primaryCommandPool, nullptr);
    vkDestroyFence(mDevice->getHandle(), frame.queueSubmittedFence, nullptr);
  }

  vkDestroyImageView(mDevice->getHandle(), mDepthStencil.view, nullptr);
  vkDestroyImage(mDevice->getHandle(), mDepthStencil.image, nullptr);
  vkFreeMemory(mDevice->getHandle(), mDepthStencil.memory, nullptr);

  cleanupFramebuffers();

  for (size_t i = 0; i < mImageCount; i++) {
    vkDestroyImageView(mDevice->getHandle(), mImageViews[i], nullptr);
  }

  vkDestroySwapchainKHR(mDevice->getHandle(), mHandle, nullptr);
}

void Swapchain::initFrame(Frame &frame) {
  VkFenceCreateInfo fenceCreateInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
  fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  vkOK(
      vkCreateFence(mDevice->getHandle(), &fenceCreateInfo, nullptr, &frame.queueSubmittedFence));

  VkCommandPoolCreateInfo commandPoolCreateInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  commandPoolCreateInfo.flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
  commandPoolCreateInfo.queueFamilyIndex = mDevice->getQueueFamilyIndices().graphics;
  vkOK(vkCreateCommandPool(mDevice->getHandle(), &commandPoolCreateInfo, nullptr,
                               &frame.primaryCommandPool));

  VkCommandBufferAllocateInfo commandBufferAllocateInfo{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  commandBufferAllocateInfo.commandPool        = frame.primaryCommandPool;
  commandBufferAllocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  commandBufferAllocateInfo.commandBufferCount = 1;
  vkOK(vkAllocateCommandBuffers(mDevice->getHandle(), &commandBufferAllocateInfo,
                                    &frame.primaryCommandBuffer));
}

void Swapchain::createFramebuffers(const std::shared_ptr<RenderPass> &renderPass) {
  VkImageView attachments[2];
  attachments[1] = mDepthStencil.view; // Depth/Stencil attachment is the same for all framebuffers

  // Build the framebuffer.
  VkFramebufferCreateInfo framebufferCreateInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
  framebufferCreateInfo.renderPass      = renderPass->getHandle();
  framebufferCreateInfo.attachmentCount = 2;
  framebufferCreateInfo.pAttachments    = attachments;
  framebufferCreateInfo.width           = mImageExtent.width;
  framebufferCreateInfo.height          = mImageExtent.height;
  framebufferCreateInfo.layers          = 1;

  for (const auto &imageView : mImageViews) {
    attachments[0] = imageView;

    VkFramebuffer framebuffer;
    vkOK(
        vkCreateFramebuffer(mDevice->getHandle(), &framebufferCreateInfo, nullptr, &framebuffer));
    mFramebuffers.push_back(framebuffer);
  }
}

void Swapchain::createDepthStencil() {
  VkImageCreateInfo imageCreateInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  imageCreateInfo.imageType   = VK_IMAGE_TYPE_2D;
  imageCreateInfo.format      = mDepthFormat;
  imageCreateInfo.extent      = {mImageExtent.width, mImageExtent.height, 1};
  imageCreateInfo.mipLevels   = 1;
  imageCreateInfo.arrayLayers = 1;
  imageCreateInfo.samples     = VK_SAMPLE_COUNT_1_BIT;
  imageCreateInfo.tiling      = VK_IMAGE_TILING_OPTIMAL;
  imageCreateInfo.usage =
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

  vkOK(vkCreateImage(mDevice->getHandle(), &imageCreateInfo, nullptr, &mDepthStencil.image));

  VkMemoryRequirements memoryRequirements{};
  vkGetImageMemoryRequirements(mDevice->getHandle(), mDepthStencil.image, &memoryRequirements);

  VkMemoryAllocateInfo memoryAllocation{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  memoryAllocation.allocationSize  = memoryRequirements.size;
  memoryAllocation.memoryTypeIndex = mDevice->getPhysicalDevice()->getMemoryType(
      memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  vkOK(
      vkAllocateMemory(mDevice->getHandle(), &memoryAllocation, nullptr, &mDepthStencil.memory));
  vkOK(vkBindImageMemory(mDevice->getHandle(), mDepthStencil.image, mDepthStencil.memory, 0));

  VkImageViewCreateInfo imageViewCreateInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  imageViewCreateInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
  imageViewCreateInfo.image                           = mDepthStencil.image;
  imageViewCreateInfo.format                          = mDepthFormat;
  imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
  imageViewCreateInfo.subresourceRange.levelCount     = 1;
  imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
  imageViewCreateInfo.subresourceRange.layerCount     = 1;
  imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
  // Stencil aspect should only be set on depth + stencil formats: VK_FORMAT_D16_UNORM_S8_UINT,
  // VK_FORMAT_D32_SFLOAT_S8_UINT
  if (mDepthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
    imageViewCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
  }
  vkOK(
      vkCreateImageView(mDevice->getHandle(), &imageViewCreateInfo, nullptr, &mDepthStencil.view));
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
    VkSemaphoreCreateInfo semaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    vkOK(vkCreateSemaphore(mDevice->getHandle(), &semaphoreCreateInfo, nullptr, &semaphore));
  } else {
    semaphore = mSemaphorePool.back();
    mSemaphorePool.pop_back();
  }

  auto result = vkAcquireNextImageKHR(mDevice->getHandle(), mHandle, UINT64_MAX, semaphore,
                                      VK_NULL_HANDLE, &imageIndex);
  if (result != VK_SUCCESS) {
    mSemaphorePool.push_back(semaphore);
    return result;
  }

  auto &frame = mFrames[imageIndex];

  // If we have outstanding fence for this swapchain imageIndex, wait for it
  // to complete first. After begin frame returns, it is safe to reuse or delete
  // resources which were used previously.
  //
  // We wait for fence which completes N frames earlier, so we do not stall,
  // waiting for all GPU work to complete before this returns.
  // Normally, this doesn't really block at all, since we're waiting for old
  // frames to have been completed, but just in case.
  if (frame.queueSubmittedFence != VK_NULL_HANDLE) {
    vkWaitForFences(mDevice->getHandle(), 1, &frame.queueSubmittedFence, true, UINT64_MAX);
    vkResetFences(mDevice->getHandle(), 1, &frame.queueSubmittedFence);
  }

  if (frame.primaryCommandPool != VK_NULL_HANDLE) {
    vkResetCommandPool(mDevice->getHandle(), frame.primaryCommandPool, 0);
  }

  // Recycle the old semaphore back into the semaphore pool.
  if (frame.acquiredSemaphore != VK_NULL_HANDLE) {
    mSemaphorePool.push_back(frame.acquiredSemaphore);
  }

  frame.acquiredSemaphore = semaphore;

  return VK_SUCCESS;
}
