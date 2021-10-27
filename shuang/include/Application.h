#pragma once

#include "Camera.h"
#include "DescriptorPool.h"
#include "DescriptorSetLayout.h"
#include "Device.h"
#include "IndexBuffer.h"
#include "InputEvent.h"
#include "Instance.h"
#include "Logger.h"
#include "PhysicalDevice.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "Surface.h"
#include "Swapchain.h"
#include "VertexBuffer.h"
#include "Window.h"

class Application {
public:
  Application();
  virtual ~Application();

  bool setup(bool enableValidation = true);
  void mainLoop();
  void handleEvent(const InputEvent &inputEvent);
  void resize(int width, int height);

protected:
  virtual void     update();
  virtual VkResult render(uint32_t imageIndex);
  virtual VkResult present(uint32_t imageIndex);

  std::string mTitle  = "Example";
  uint32_t    mWidth  = 480;
  uint32_t    mHeight = 360;
  Camera      mCamera;

private:
  // prepare data: vertex buffer
  virtual void prepare();

  std::shared_ptr<Window>              mWindow              = nullptr;
  std::shared_ptr<Instance>            mInstance            = nullptr;
  std::shared_ptr<PhysicalDevice>      mPhysicalDevice      = nullptr;
  std::shared_ptr<Surface>             mSurface             = nullptr;
  std::shared_ptr<Device>              mDevice              = nullptr;
  std::shared_ptr<Swapchain>           mSwapchain           = nullptr;
  std::shared_ptr<RenderPass>          mRenderPass          = nullptr;
  std::shared_ptr<DescriptorSetLayout> mDescriptorSetLayout = nullptr;
  //  std::shared_ptr<DescriptorPool>      mDescriptorPool      = nullptr;
  std::shared_ptr<Pipeline>     mPipeline     = nullptr;
  std::shared_ptr<VertexBuffer> mVertexBuffer = nullptr;
  std::shared_ptr<IndexBuffer>  mIndexBuffer  = nullptr;
};
