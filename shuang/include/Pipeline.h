#pragma once

#include <vulkan/vulkan.hpp>

class Device;
class RenderPass;

class Pipeline {
public:
  Pipeline(const std::shared_ptr<Device>     &device,
           const std::shared_ptr<RenderPass> &renderPass);
  ~Pipeline();

  const VkPipeline &getHandle() const { return mHandle; }

private:
  VkShaderModule createShaderModule(const char *path);

  const std::shared_ptr<Device> &mDevice = nullptr;
  VkPipelineLayout               mPipelineLayout;
  VkPipeline                     mHandle;
};
