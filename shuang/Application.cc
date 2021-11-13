#include "Application.h"
#include "FreeCamera.h"
#include "Macros.h"
#include "OrbitCamera.h"
#include "Vertex.h"

#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

struct alignas(16) vs_ubo_t {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

Application::Application(const Setting &setting) { spdlog::set_level(spdlog::level::debug); }

Application::~Application() { log_func; }

void Application::handleEvent(const InputEvent &inputEvent) {
  if (inputEvent.getSource() == InputEventSource::KEYBOARD) {
    const auto &event = static_cast<const KeyInputEvent &>(inputEvent);
    if (event.getAction() == KeyAction::UP && event.getCode() == KeyCode::ESCAPE) {
      return mWindow->close();
    }
  }

  mCamera->handleEvent(inputEvent);
}

void Application::resize(const int width, const int height) {
  VkSurfaceCapabilitiesKHR capabilities;
  vkAssert(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mDevice->getPhysicalDevice()->getHandle(),
                                                     mSurface->getHandle(), &capabilities));
  if (capabilities.currentExtent.width == mSwapchain->getImageExtent().width &&
      capabilities.currentExtent.height == mSwapchain->getImageExtent().height) {
    return;
  }

  mDevice->waitIdle();

  mSwapchain.reset();
  mSwapchain = std::make_shared<Swapchain>(mDevice, mSurface);
  mSwapchain->createDepthStencil();
  mSwapchain->createFramebuffers(mRenderPass);
}

void Application::setFocus(bool focus) { mWindow->setFocused(focus); }

void Application::mainLoop() {
  while (!mWindow->shouldClose()) {
    static int  frames   = 0;
    static auto lastTime = mWindow->getTime();

    { // update
      static auto lastTime    = mWindow->getTime();
      auto        currentTime = mWindow->getTime();
      update(static_cast<float>(currentTime - lastTime));
      lastTime = currentTime;
    }

    mWindow->pollEvents();

    if (++frames >= 60) {
      auto currentTime = mWindow->getTime();
      auto deltaTime   = currentTime - lastTime;
      auto fps         = frames / deltaTime;

      log_debug("FPS {:.2f} {:.4f} ms", fps, deltaTime / frames * 1e3);

      frames   = 0;
      lastTime = currentTime;
    }
  }
  mDevice->waitIdle();
}

bool Application::setup(bool enableValidation) {
  const std::vector<const char *> extensions{};
  mInstance       = std::make_shared<Instance>(mTitle.c_str(), extensions, enableValidation);
  mWindow         = std::make_unique<Window>(this, mWidth, mHeight, mTitle.c_str());
  mSurface        = std::make_shared<Surface>(mInstance, mWindow);
  mPhysicalDevice = std::make_shared<PhysicalDevice>(mInstance);
  mDevice         = std::make_shared<Device>(mPhysicalDevice, mSurface);
  mSwapchain      = std::make_shared<Swapchain>(mDevice, mSurface);
  auto imageCount = mSwapchain->getImageCount();
  mRenderPass     = std::make_shared<RenderPass>(mDevice, mSwapchain->getImageFormat(),
                                             mSwapchain->getDepthFormat());
  mSwapchain->createDepthStencil();
  mSwapchain->createFramebuffers(mRenderPass);

  //  mCamera = std::make_shared<FreeCamera>();
  mCamera = std::make_shared<OrbitCamera>();
  mCamera->setPerspective(60.0f, (float)mWidth / (float)mHeight, 0.5f, 50.0f);

  initializeModels();

  mDescriptorSetLayout = std::make_shared<DescriptorSetLayout>(mDevice);

  auto setLayouts = {mDescriptorSetLayout->getHandle()};
  mPipeline       = std::make_shared<Pipeline>(mDevice, mRenderPass, setLayouts);
  mDescriptorPool = std::make_shared<DescriptorPool>(mDevice, imageCount, imageCount);

  auto bufferInfo = createDescriptorBufferInfo(mUniformBuffer->getHandle(), sizeof(vs_ubo_t));
  mDescriptorSet =
      std::make_shared<DescriptorSet>(mDevice, mDescriptorPool, 1, setLayouts, bufferInfo);

  return true;
}

void Application::initializeModels() {
  mModels.push_back(std::make_unique<Cube>(mDevice));
  mModels.push_back(std::make_unique<Triangle>(mDevice));

  // Uniform buffer
  mUniformBuffer = std::make_unique<UniformBuffer>(
      mDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      sizeof(vs_ubo_t));

  updateUniformBuffer();
}

void Application::updateUniformBuffer() {
  static auto startTime   = std::chrono::high_resolution_clock::now();
  auto        currentTime = std::chrono::high_resolution_clock::now();
  auto        time =
      std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

  vs_ubo_t ubo{};
  ubo.model = glm::mat4(1.f);
  ubo.view  = mCamera->getViewMatrix();
  ubo.proj  = mCamera->getProjectionMatrix();

  mUniformBuffer->copy(&ubo, sizeof(ubo));
}

void Application::update(float timeStep) {
  uint32_t imageIndex;

  auto result = mSwapchain->acquireNextImage(imageIndex);
  if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) {
    resize(mSwapchain->getImageExtent().width, mSwapchain->getImageExtent().height);
    result = mSwapchain->acquireNextImage(imageIndex);
  }

  if (result != VK_SUCCESS) {
    vkQueueWaitIdle(mDevice->getGraphicsQueue());
    return;
  }

  updateUniformBuffer();

  updateScene(timeStep);

  vkAssert(render(imageIndex));
  vkAssert(present(imageIndex));
}

void Application::updateScene(float timeStep) { mCamera->update(timeStep); }

VkResult Application::render(const uint32_t imageIndex) {
  auto &frame = mSwapchain->getFrames()[imageIndex];

  // Render to this framebuffer.
  auto &framebuffer = mSwapchain->getFramebuffers()[imageIndex];

  // Allocate or re-use a primary command buffer.
  auto commandBuffer = frame.primaryCommandBuffer;

  // We will only submit this once before it's recycled.
  VkCommandBufferBeginInfo commandBufferBeginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  // Begin command recording
  vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

  // Set clear color values.
  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color        = {{0.0f, 0.0f, 0.0f, 1.0f}};
  clearValues[1].depthStencil = {1.0f, 0};

  // Begin the render pass.
  VkRenderPassBeginInfo renderPassBeginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
  renderPassBeginInfo.renderPass               = mRenderPass->getHandle();
  renderPassBeginInfo.framebuffer              = framebuffer;
  renderPassBeginInfo.renderArea.extent.width  = mSwapchain->getImageExtent().width;
  renderPassBeginInfo.renderArea.extent.height = mSwapchain->getImageExtent().height;
  renderPassBeginInfo.clearValueCount          = clearValues.size();
  renderPassBeginInfo.pClearValues             = clearValues.data();
  // We will add draw commands in the same command buffer.
  vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline->getLayout(), 0,
                          1, &mDescriptorSet->getHandle(), 0, nullptr);

  // Bind the graphics pipeline.
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline->getHandle());

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

  for (const auto &model : mModels) {
    drawModel(commandBuffer, model.get());
  }

  // Complete render pass.
  vkCmdEndRenderPass(commandBuffer);

  // Complete the command buffer.
  vkAssert(vkEndCommandBuffer(commandBuffer));

  // Submit it to the queue with a release semaphore.
  if (frame.releasedSemaphore == VK_NULL_HANDLE) {
    VkSemaphoreCreateInfo semaphoreCreateInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    vkAssert(vkCreateSemaphore(mDevice->getHandle(), &semaphoreCreateInfo, nullptr,
                               &frame.releasedSemaphore));
  }

  VkPipelineStageFlags waitStage{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

  VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submitInfo.commandBufferCount   = 1;
  submitInfo.pCommandBuffers      = &commandBuffer;
  submitInfo.waitSemaphoreCount   = 1;
  submitInfo.pWaitSemaphores      = &frame.acquiredSemaphore;
  submitInfo.pWaitDstStageMask    = &waitStage;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores    = &frame.releasedSemaphore;
  // Submit command buffer to graphics queue
  return vkQueueSubmit(mDevice->getGraphicsQueue(), 1, &submitInfo, frame.queueSubmittedFence);
}

VkResult Application::present(const uint32_t imageIndex) {
  auto &frame = mSwapchain->getFrames()[imageIndex];

  VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
  presentInfo.swapchainCount     = 1;
  presentInfo.pSwapchains        = &mSwapchain->getHandle();
  presentInfo.pImageIndices      = &imageIndex;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores    = &frame.releasedSemaphore;
  // Present swapchain imageIndex
  vkAssert(vkQueuePresentKHR(mDevice->getGraphicsQueue(), &presentInfo));
  return vkQueueWaitIdle(mDevice->getGraphicsQueue());
}

VkDescriptorBufferInfo Application::createDescriptorBufferInfo(VkBuffer buffer, VkDeviceSize range,
                                                               VkDeviceSize offset) {
  VkDescriptorBufferInfo descriptorBufferInfo{};
  descriptorBufferInfo.buffer = buffer;
  descriptorBufferInfo.range  = range;
  descriptorBufferInfo.offset = offset;
  return descriptorBufferInfo;
}

void Application::drawModel(const VkCommandBuffer &commandBuffer, const Model *model) {
  VkDeviceSize offsets[1] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model->getVertexBuffer()->getHandle(), offsets);
  vkCmdBindIndexBuffer(commandBuffer, model->getIndexBuffer()->getHandle(), 0,
                       VK_INDEX_TYPE_UINT32);
  vkCmdDrawIndexed(commandBuffer, model->getIndexBuffer()->getIndexCount(), 1, 0, 0, 0);
  //  vkCmdDraw(commandBuffer, 36, 1, 0, 0);
}
