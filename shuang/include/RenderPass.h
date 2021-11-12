#pragma once

#include <vulkan/vulkan.hpp>

class Device;

class RenderPass {
public:
  RenderPass(const std::shared_ptr<Device> &device, VkFormat imageFormat, VkFormat depthFormat);
  ~RenderPass();

  const VkRenderPass &getHandle() const { return mHandle; }

private:
  const std::shared_ptr<Device> &mDevice = nullptr;
  VkRenderPass                   mHandle;
};
