#pragma once

#include "Camera.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "DescriptorSetLayout.h"
#include "Device.h"
#include "IndexBuffer.h"
#include "InputEvent.h"
#include "Instance.h"
#include "Log.h"
#include "PhysicalDevice.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "Surface.h"
#include "Swapchain.h"
#include "UniformBuffer.h"
#include "VertexBuffer.h"
#include "Window.h"
#include "model/Model.h"

class Application {
public:
  struct Setting {};

  explicit Application(const Setting &setting = {});
  virtual ~Application();

  bool setup(bool enableValidation = true);
  void setupDescriptorSetLayouts();
  void setupPipelines();
  void mainLoop();
  void handleEvent(const InputEvent &inputEvent);
  void resize(int width, int height);
  void setFocus(bool focus);

protected:
  virtual void     update(float timeStep);
  void             updateScene(float timeStep);
  virtual VkResult render(uint32_t imageIndex);
  virtual VkResult present(uint32_t imageIndex);

  std::string mTitle  = "Example";
  uint32_t    mWidth  = 480;
  uint32_t    mHeight = 360;

private:
  // load models; create uniform buffer
  virtual void                    setupModels();
  void                            updateUniformBuffer();
  VkShaderModule                  loadShader(const char *path);
  VkPipelineShaderStageCreateInfo loadShader(const char *path, VkShaderStageFlagBits stage);
  static void                     draw(const VkCommandBuffer &commandBuffer, const Model *model);

  std::shared_ptr<Window>         mWindow         = nullptr;
  std::shared_ptr<Instance>       mInstance       = nullptr;
  std::shared_ptr<PhysicalDevice> mPhysicalDevice = nullptr;
  std::shared_ptr<Surface>        mSurface        = nullptr;
  std::shared_ptr<Device>         mDevice         = nullptr;
  std::shared_ptr<Swapchain>      mSwapchain      = nullptr;
  std::shared_ptr<RenderPass>     mRenderPass     = nullptr;
  struct {
    std::unique_ptr<DescriptorSetLayout> model;
  } mDescriptorSetLayouts;
  struct {
    std::unique_ptr<DescriptorSet> model;
  } mDescriptorSets;
  std::shared_ptr<DescriptorPool> mDescriptorPool = nullptr;
  struct {
    VkPipelineLayout model;
  } mPipelineLayouts;
  struct {
    VkPipeline grid; // line mode
    VkPipeline model; // triangle mode
  } mPipelines;
  std::unique_ptr<UniformBuffer> mUniformBuffer = nullptr;
  struct {
    std::unique_ptr<Model> grid;
    std::unique_ptr<Model> cube;
  } mModels;
  std::shared_ptr<Camera> mCamera;
};
