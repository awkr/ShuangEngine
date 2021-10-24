#pragma once

#include <vulkan/vulkan.hpp>

class Device;
class RenderPass;

class Pipeline {
public:
  Pipeline(const Device *device, const RenderPass *renderPass);
  ~Pipeline();

  const VkPipeline &getHandle() const { return mHandle; }

private:
  VkShaderModule createShaderModule(const char *path);

  const Device    *mDevice;
  VkPipelineLayout mPipelineLayout;
  VkPipeline       mHandle;
};
