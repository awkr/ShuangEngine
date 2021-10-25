#include "Application.h"
#include "Macros.h"

#include <spdlog/spdlog.h>

Application::Application() { spdlog::set_level(spdlog::level::debug); }

Application::~Application() { log_func; }

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

  mDevice->waitIdle();

  mSwapchain.reset();
  mSwapchain = std::make_shared<Swapchain>(mDevice, mSurface);
  mSwapchain->createFramebuffers(mRenderPass);
}

void Application::mainLoop() {
  while (!mWindow->shouldClose()) {
    update();
    mWindow->pollEvents();
  }
  mDevice->waitIdle();
}

bool Application::setup(bool enableValidation) {
  const std::vector<const char *> extensions{};
  mInstance =
      std::make_shared<Instance>(mTitle.c_str(), extensions, enableValidation);
  mWindow  = std::make_unique<Window>(this, mWidth, mHeight, mTitle.c_str());
  mSurface = std::make_shared<Surface>(mInstance, mWindow);
  mPhysicalDevice = std::make_shared<PhysicalDevice>(mInstance);
  mDevice         = std::make_shared<Device>(mPhysicalDevice, mSurface);
  mSwapchain      = std::make_shared<Swapchain>(mDevice, mSurface);
  mRenderPass =
      std::make_shared<RenderPass>(mDevice, mSwapchain->getImageFormat());
  mPipeline = std::make_shared<Pipeline>(mDevice, mRenderPass);

  mSwapchain->createFramebuffers(mRenderPass);

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
  auto &framebuffer = mSwapchain->getFramebuffers()[imageIndex];

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
  // Set viewport dynamically
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.extent = mSwapchain->getImageExtent();
  // Set scissor dynamically
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
