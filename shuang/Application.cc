#include "Application.h"
#include "FileSystem.h"
#include "FreeCamera.h"
#include "Initializer.h"
#include "Macros.h"
#include "OrbitCamera.h"
#include "Vertex.h"
#include "model/Cube.h"
#include "model/Grid.h"
#include "model/Triangle.h"

#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

struct alignas(16) vs_ubo_t {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

Application::Application(const Setting &setting) { spdlog::set_level(spdlog::level::debug); }

Application::~Application() {
  log_func;

  vkDestroyPipeline(mDevice->getHandle(), mPipelines.grid, nullptr);
  vkDestroyPipeline(mDevice->getHandle(), mPipelines.model, nullptr);
  vkDestroyPipelineLayout(mDevice->getHandle(), mPipelineLayouts.model, nullptr);
}

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
  vkOK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mDevice->getPhysicalDevice()->getHandle(),
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

  setupModels();

  setupDescriptorSetLayouts();
  setupPipelines();
  mDescriptorPool = std::make_shared<DescriptorPool>(mDevice, imageCount, imageCount);

  auto setLayouts = {mDescriptorSetLayouts.model->getHandle()};
  auto bufferInfo = createDescriptorBufferInfo(mUniformBuffer->getHandle(), sizeof(vs_ubo_t));
  mDescriptorSets.model =
      std::make_unique<DescriptorSet>(mDevice, mDescriptorPool, 1, setLayouts, bufferInfo);

  return true;
}

void Application::setupDescriptorSetLayouts() {
  mDescriptorSetLayouts.model = std::make_unique<DescriptorSetLayout>(mDevice);
}

void Application::setupPipelines() {
  { // Create pipeline layout
    std::vector<VkDescriptorSetLayout> setLayouts{mDescriptorSetLayouts.model->getHandle()};

    VkPipelineLayoutCreateInfo createInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    createInfo.pSetLayouts    = setLayouts.data();
    createInfo.setLayoutCount = setLayouts.size();
    vkOK(vkCreatePipelineLayout(mDevice->getHandle(), &createInfo, nullptr,
                                &mPipelineLayouts.model));
  }

  // Vertex binding and attributes

  // Binding descriptions
  VkVertexInputBindingDescription binding{
      .binding   = 0,
      .stride    = sizeof(Vertex),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
  };
  std::vector<VkVertexInputBindingDescription> bindings = {
      binding,
  };

  // Attribute descriptions
  VkVertexInputAttributeDescription attribute_position = {
      .location = 0,
      .binding  = 0,
      .format   = VK_FORMAT_R32G32B32_SFLOAT,
      .offset   = offsetof(Vertex, position),
  };
  VkVertexInputAttributeDescription attribute_color = {
      .location = 1,
      .binding  = 0,
      .format   = VK_FORMAT_R32G32B32_SFLOAT,
      .offset   = offsetof(Vertex, color),
  };
  std::vector<VkVertexInputAttributeDescription> attributes = {
      attribute_position,
      attribute_color,
  };

  VkPipelineVertexInputStateCreateInfo vertexInputState{
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
  vertexInputState.pVertexBindingDescriptions      = bindings.data();
  vertexInputState.vertexBindingDescriptionCount   = static_cast<uint32_t>(bindings.size());
  vertexInputState.pVertexAttributeDescriptions    = attributes.data();
  vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
  inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

  // Specify rasterization state.
  VkPipelineRasterizationStateCreateInfo rasterizationState{
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
  rasterizationState.cullMode  = VK_CULL_MODE_NONE;
  rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizationState.lineWidth = 1.0f;

  // Our attachment will write to all color channels, but no blending is enabled.
  VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
  colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                             VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo colorBlendState{
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
  colorBlendState.attachmentCount = 1;
  colorBlendState.pAttachments    = &colorBlendAttachmentState;

  // We will have one viewportState and scissor box.
  VkPipelineViewportStateCreateInfo viewportState{
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
  viewportState.viewportCount = 1;
  viewportState.scissorCount  = 1;

  VkPipelineDepthStencilStateCreateInfo depthStencilState{
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  depthStencilState.depthTestEnable       = VK_TRUE;
  depthStencilState.depthWriteEnable      = VK_TRUE;
  depthStencilState.depthCompareOp        = VK_COMPARE_OP_LESS;
  depthStencilState.depthBoundsTestEnable = VK_FALSE;
  depthStencilState.minDepthBounds        = 0.0f;
  depthStencilState.maxDepthBounds        = 1.0f;
  depthStencilState.stencilTestEnable     = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampleState{
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
  multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // No multisampling

  // Specify that these states will be dynamic, i.e. not part of pipeline state object.
  std::array<VkDynamicState, 2> dynamicStates{
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
  };

  VkPipelineDynamicStateCreateInfo dynamicState{
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
  dynamicState.pDynamicStates    = dynamicStates.data();
  dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());

  // Load SPIR-V shaders
  std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{
      loadShader("shaders/triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
      loadShader("shaders/triangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT),
  };

  VkGraphicsPipelineCreateInfo createInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
  createInfo.stageCount          = static_cast<uint32_t>(shaderStages.size());
  createInfo.pStages             = shaderStages.data();
  createInfo.pVertexInputState   = &vertexInputState;
  createInfo.pInputAssemblyState = &inputAssemblyState;
  createInfo.pRasterizationState = &rasterizationState;
  createInfo.pColorBlendState    = &colorBlendState;
  createInfo.pMultisampleState   = &multisampleState;
  createInfo.pViewportState      = &viewportState;
  createInfo.pDepthStencilState  = &depthStencilState;
  createInfo.pDynamicState       = &dynamicState;
  createInfo.renderPass          = mRenderPass->getHandle();
  createInfo.layout              = mPipelineLayouts.model;

  vkOK(vkCreateGraphicsPipelines(mDevice->getHandle(), VK_NULL_HANDLE, 1, &createInfo, nullptr,
                                 &mPipelines.grid));

  inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  vkOK(vkCreateGraphicsPipelines(mDevice->getHandle(), VK_NULL_HANDLE, 1, &createInfo, nullptr,
                                 &mPipelines.model));

  // Pipeline is baked, we can delete the shader modules now.
  vkDestroyShaderModule(mDevice->getHandle(), shaderStages[0].module, nullptr);
  vkDestroyShaderModule(mDevice->getHandle(), shaderStages[1].module, nullptr);
}

void Application::setupModels() {
  mModels.grid = std::make_unique<Grid>(mDevice, 5);
  mModels.cube = std::make_unique<Cube>(mDevice);
  //  mModels.push_back(std::make_unique<Triangle>(mDevice));

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

  vkOK(render(imageIndex));
  vkOK(present(imageIndex));
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
  clearValues[0].color        = {{1.0f, 1.0f, 1.0f, 1.0f}};
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

  // Draw grid
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelines.grid);
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayouts.model, 0,
                          1, &mDescriptorSets.model->getHandle(), 0, nullptr);

  draw(commandBuffer, mModels.grid.get());

  // Draw model
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelines.model);
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayouts.model, 0,
                          1, &mDescriptorSets.model->getHandle(), 0, nullptr);

  draw(commandBuffer, mModels.cube.get());

  // Complete render pass.
  vkCmdEndRenderPass(commandBuffer);

  // Complete the command buffer.
  vkOK(vkEndCommandBuffer(commandBuffer));

  // Submit it to the queue with a release semaphore.
  if (frame.releasedSemaphore == VK_NULL_HANDLE) {
    VkSemaphoreCreateInfo semaphoreCreateInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    vkOK(vkCreateSemaphore(mDevice->getHandle(), &semaphoreCreateInfo, nullptr,
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
  vkOK(vkQueuePresentKHR(mDevice->getGraphicsQueue(), &presentInfo));
  return vkQueueWaitIdle(mDevice->getGraphicsQueue());
}

VkShaderModule Application::loadShader(const char *path) {
  auto source = filesystem::read(path);

  VkShaderModuleCreateInfo createInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
  createInfo.codeSize = source.size();
  createInfo.pCode    = reinterpret_cast<const uint32_t *>(source.data());

  VkShaderModule shaderModule;
  vkOK(vkCreateShaderModule(mDevice->getHandle(), &createInfo, nullptr, &shaderModule));
  return shaderModule;
}

VkPipelineShaderStageCreateInfo Application::loadShader(const char           *path,
                                                        VkShaderStageFlagBits stage) {
  VkPipelineShaderStageCreateInfo createInfo{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
  createInfo.stage  = stage;
  createInfo.module = loadShader(path);
  createInfo.pName  = "main";
  return createInfo;
}

void Application::draw(const VkCommandBuffer &commandBuffer, const Model *model) {
  VkDeviceSize offsets[1] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model->getVertexBuffer()->getHandle(), offsets);
  vkCmdBindIndexBuffer(commandBuffer, model->getIndexBuffer()->getHandle(), 0,
                       VK_INDEX_TYPE_UINT32);
  vkCmdDrawIndexed(commandBuffer, model->getIndexBuffer()->getIndexCount(), 1, 0, 0, 0);
  //  vkCmdDraw(commandBuffer, model->getVertexBuffer()->getCount(), 1, 0, 0);
}
