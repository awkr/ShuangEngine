#include "Application.h"
#include "Macros.h"

Application::Application() {}

Application::~Application() {
  logInfo(__func__);

  cleanupFramebuffers();
  delete mPipeline;
  delete mRenderPass;
  delete mSwapchain;
  delete mSurface;
  delete mDevice;
  delete mInstance;
}

void Application::handleEvent(const InputEvent &inputEvent) {
  if (inputEvent.getSource() == InputEventSource::KEYBOARD) {
    const auto &keyInputEvent = static_cast<const KeyInputEvent &>(inputEvent);
    if (keyInputEvent.getCode() == KeyCode::ESCAPE) {
      mWindow->close();
    }
  }
}

void Application::resize(const int width, const int height) {
  VkSurfaceCapabilitiesKHR capabilities;
  ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      mDevice->getPhysicalDevice()->getHandle(), mSurface->getHandle(),
      &capabilities));
  if (capabilities.currentExtent.width == mSwapchain->getImageExtent().width &&
      capabilities.currentExtent.height ==
          mSwapchain->getImageExtent().height) {
    return;
  }

  vkDeviceWaitIdle(mDevice->getHandle());

  cleanupFramebuffers();

  delete mSwapchain;
  mSwapchain = new Swapchain(mDevice, mSurface);

  createFramebuffers();
}

void Application::mainLoop() {
  while (!mWindow->shouldClose()) {
    update();
    mWindow->pollEvents();
  }
  vkDeviceWaitIdle(mDevice->getHandle());
}

bool Application::setup(bool enableValidation) {
  mInstance = new Instance(mTitle.c_str(), enableValidation);
  mWindow   = std::make_unique<Window>(this, mWidth, mHeight, mTitle.c_str());
  mSurface  = new Surface(mInstance, mWindow);
  mPhysicalDevice = new PhysicalDevice(mInstance);
  mDevice         = new Device(mPhysicalDevice, mSurface);
  mSwapchain      = new Swapchain(mDevice, mSurface);
  mRenderPass     = new RenderPass(mDevice, mSwapchain->getImageFormat());
  mPipeline       = new Pipeline(mDevice, mRenderPass);

  createFramebuffers();

  return true;
}

void Application::update() {
  uint32_t imageIndex;

  auto result = mSwapchain->acquireNextImage(imageIndex);
  if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) {
    resize(mSwapchain->getImageExtent().width,
           mSwapchain->getImageExtent().height);
    result = mSwapchain->acquireNextImage(imageIndex);
  }

  if (result != VK_SUCCESS) {
    vkQueueWaitIdle(mDevice->getGraphicsQueue());
    return;
  }

  ASSERT(render(imageIndex));
  ASSERT(present(imageIndex));
}

VkResult Application::render(const uint32_t imageIndex) {
  auto &frameClip = mSwapchain->getFrameClips()[imageIndex];

  // Render to this framebuffer.
  auto framebuffer = mFramebuffers[imageIndex];

  // Allocate or re-use a primary command buffer.
  auto commandBuffer = frameClip.primaryCommandBuffer;

  // We will only submit this once before it's recycled.
  VkCommandBufferBeginInfo commandBufferBeginInfo{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  // Begin command recording
  vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

  // Set clear color values.
  VkClearValue clearValue;
  clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};

  // Begin the render pass.
  VkRenderPassBeginInfo renderPassBeginInfo{
      VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
  renderPassBeginInfo.renderPass  = mRenderPass->getHandle();
  renderPassBeginInfo.framebuffer = framebuffer;
  renderPassBeginInfo.renderArea.extent.width =
      mSwapchain->getImageExtent().width;
  renderPassBeginInfo.renderArea.extent.height =
      mSwapchain->getImageExtent().height;
  renderPassBeginInfo.clearValueCount = 1;
  renderPassBeginInfo.pClearValues    = &clearValue;
  // We will add draw commands in the same command buffer.
  vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  // Bind the graphics pipeline.
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    mPipeline->getHandle());

  VkViewport viewport{};
  viewport.width    = static_cast<float>(mSwapchain->getImageExtent().width);
  viewport.height   = static_cast<float>(mSwapchain->getImageExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.extent.width  = mSwapchain->getImageExtent().width;
  scissor.extent.height = mSwapchain->getImageExtent().height;
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  // Draw three vertices with one instance.
  vkCmdDraw(commandBuffer, 3, 1, 0, 0);

  // Complete render pass.
  vkCmdEndRenderPass(commandBuffer);

  // Complete the command buffer.
  ASSERT(vkEndCommandBuffer(commandBuffer));

  // Submit it to the queue with a release semaphore.
  if (frameClip.releasedSemaphore == VK_NULL_HANDLE) {
    VkSemaphoreCreateInfo semaphoreCreateInfo{
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    ASSERT(vkCreateSemaphore(mDevice->getHandle(), &semaphoreCreateInfo,
                             nullptr, &frameClip.releasedSemaphore));
  }

  VkPipelineStageFlags waitStage{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

  VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submitInfo.commandBufferCount   = 1;
  submitInfo.pCommandBuffers      = &commandBuffer;
  submitInfo.waitSemaphoreCount   = 1;
  submitInfo.pWaitSemaphores      = &frameClip.acquiredSemaphore;
  submitInfo.pWaitDstStageMask    = &waitStage;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores    = &frameClip.releasedSemaphore;
  // Submit command buffer to graphics queue
  return vkQueueSubmit(mDevice->getGraphicsQueue(), 1, &submitInfo,
                       frameClip.queueSubmittedFence);
}

VkResult Application::present(const uint32_t imageIndex) {
  auto &frameClip = mSwapchain->getFrameClips()[imageIndex];

  VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
  presentInfo.swapchainCount     = 1;
  presentInfo.pSwapchains        = &mSwapchain->getHandle();
  presentInfo.pImageIndices      = &imageIndex;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores    = &frameClip.releasedSemaphore;
  // Present swapchain imageIndex
  return vkQueuePresentKHR(mDevice->getGraphicsQueue(), &presentInfo);
}

// Create framebuffer for each swapchain image view
void Application::createFramebuffers() {
  for (const auto &imageView : mSwapchain->getImageViews()) {
    // Build the framebuffer.
    VkFramebufferCreateInfo framebufferCreateInfo{
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    framebufferCreateInfo.renderPass      = mRenderPass->getHandle();
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.pAttachments    = &imageView;
    framebufferCreateInfo.width           = mSwapchain->getImageExtent().width;
    framebufferCreateInfo.height          = mSwapchain->getImageExtent().height;
    framebufferCreateInfo.layers          = 1;

    VkFramebuffer framebuffer;
    ASSERT(vkCreateFramebuffer(mDevice->getHandle(), &framebufferCreateInfo,
                               nullptr, &framebuffer));
    mFramebuffers.push_back(framebuffer);
  }
}

void Application::cleanupFramebuffers() {
  vkQueueWaitIdle(mDevice->getGraphicsQueue());

  for (auto &framebuffer : mFramebuffers) {
    vkDestroyFramebuffer(mDevice->getHandle(), framebuffer, nullptr);
  }

  mFramebuffers.clear();
}
