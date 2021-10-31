#include "Application.h"
#include "Macros.h"
#include "Vertex.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <spdlog/spdlog.h>

struct vs_ubo_t {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

Application::Application() { spdlog::set_level(spdlog::level::debug); }

Application::~Application() { log_func; }

float xOffset = 0;
float xSpeed  = .1;
float ySpeed  = .1;

void Application::handleEvent(const InputEvent &inputEvent) {
  if (inputEvent.getSource() == InputEventSource::KEYBOARD) {
    const auto &keyInputEvent = static_cast<const KeyInputEvent &>(inputEvent);
    switch (auto code = keyInputEvent.getCode(); code) {
    case KeyCode::ESCAPE:
      mWindow->close();
      break;

    case KeyCode::LEFT: // move camera horizontally
      xOffset -= .1;
      break;
    case KeyCode::RIGHT:
      xOffset += .1;
      break;

    case KeyCode::UP: // move camera vertically
    case KeyCode::DOWN:
      break;

    default:
      log_warn("What should I do: key #{}, action #{} ?", code, keyInputEvent.getAction());
    }
  }
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
  mSwapchain->createFramebuffers(mRenderPass);
}

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
  mRenderPass     = std::make_shared<RenderPass>(mDevice, mSwapchain->getImageFormat());
  mSwapchain->createFramebuffers(mRenderPass);

  initializeBuffers();

  mDescriptorSetLayout = std::make_shared<DescriptorSetLayout>(mDevice);

  auto setLayouts = {mDescriptorSetLayout->getHandle()};
  mPipeline       = std::make_shared<Pipeline>(mDevice, mRenderPass, setLayouts);
  mDescriptorPool = std::make_shared<DescriptorPool>(mDevice, imageCount, imageCount);

  auto bufferInfo = createDescriptorBufferInfo(mUniformBuffer->getHandle(), sizeof(vs_ubo_t));
  mDescriptorSet =
      std::make_shared<DescriptorSet>(mDevice, mDescriptorPool, 1, setLayouts, bufferInfo);

  //  mCamera = std::make_shared<Camera>();
  //  mCamera->setPosition(glm::vec3(0, 0, 2.f));
  //  mCamera->setPerspective(60.0f, (float)mWidth / (float)mHeight, 1.0f, 256.0f);

  return true;
}

void Application::initializeBuffers() {
  // Vertex buffer
  const std::vector<Vertex> vertices = {
      //      {{-1.f, -1.f, .0f}, {1.0f, 0.0f, 0.0f}},
      //      {{1.f, -1.f, .0f}, {0.0f, 1.0f, 0.0f}},
      //      {{1.f, 1.f, 0.0f}, {0.0f, 0.0f, 1.0f}},
      //      {{-1.f, 1.f, 0.0f}, {1.0f, 1.0f, 1.0f}},

      {{-.5f, .5f, .0f}, {1.0f, 0.0f, 0.0f}},
      {{.5f, .5f, .0f}, {0.0f, 1.0f, 0.0f}},
      {{.5f, -.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
  };
  auto size = vertices.size() * sizeof(Vertex);

  auto buf      = std::make_shared<Buffer>(mDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                      size, (void *)vertices.data());
  mVertexBuffer = std::make_unique<VertexBuffer>(mDevice, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, size);
  mVertexBuffer->copy(buf, mDevice->getGraphicsQueue());

  // Index buffer
  const std::vector<uint32_t> indices = {0, 1, 2};
  size                                = indices.size() * sizeof(uint32_t);

  buf.reset();
  buf = std::make_shared<Buffer>(mDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                 size, (void *)indices.data());

  mIndexBuffer =
      std::make_unique<IndexBuffer>(mDevice, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indices.size(), size);
  mIndexBuffer->copy(buf, mDevice->getGraphicsQueue());

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

  auto cameraPos    = glm::vec3(.0f + xOffset, .0f, 3.0f);
  auto cameraTarget = glm::vec3(.0f + xOffset, 0.0f, 0.0f);
  //  auto cameraDir    = glm::normalize(cameraPos - cameraTarget);
  auto cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
  //  auto cameraRight  = glm::normalize(glm::cross(cameraUp, cameraDir));
  //  cameraUp          = glm::cross(cameraDir, cameraRight);

  vs_ubo_t ubo{};
  //  ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f,
  //  0.0f, 1.0f));
  ubo.model = glm::mat4(1.f);
  ubo.view  = glm::lookAt(cameraPos, cameraTarget, cameraUp);
  ubo.proj  = glm::perspective(glm::radians(60.0f), mWidth / (float)mHeight, 0.1f, 10.0f);

  //  log_debug("view matrix: {}", glm::to_string(ubo.view));

  mUniformBuffer->copy(&ubo, sizeof(ubo));
}

void Application::update(float timeStep) {
  //  log_debug("time step {:.6f}", timeStep);

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

  vkAssert(render(imageIndex));
  vkAssert(present(imageIndex));
}

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
  VkClearValue clearValue;
  clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};

  // Begin the render pass.
  VkRenderPassBeginInfo renderPassBeginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
  renderPassBeginInfo.renderPass               = mRenderPass->getHandle();
  renderPassBeginInfo.framebuffer              = framebuffer;
  renderPassBeginInfo.renderArea.extent.width  = mSwapchain->getImageExtent().width;
  renderPassBeginInfo.renderArea.extent.height = mSwapchain->getImageExtent().height;
  renderPassBeginInfo.clearValueCount          = 1;
  renderPassBeginInfo.pClearValues             = &clearValue;
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

  VkDeviceSize offsets[1] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mVertexBuffer->getHandle(), offsets);
  vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer->getHandle(), 0, VK_INDEX_TYPE_UINT32);
  // Draw three vertices with one instance.
  vkCmdDrawIndexed(commandBuffer, mIndexBuffer->getIndexCount(), 1, 0, 0, 0);
  //  vkCmdDraw(commandBuffer, 3, 1, 0, 0);

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
  return vkQueuePresentKHR(mDevice->getGraphicsQueue(), &presentInfo);
}

VkDescriptorBufferInfo Application::createDescriptorBufferInfo(VkBuffer buffer, VkDeviceSize range,
                                                               VkDeviceSize offset) {
  VkDescriptorBufferInfo descriptorBufferInfo{};
  descriptorBufferInfo.buffer = buffer;
  descriptorBufferInfo.range  = range;
  descriptorBufferInfo.offset = offset;
  return descriptorBufferInfo;
}
