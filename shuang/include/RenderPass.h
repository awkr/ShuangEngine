#pragma once

#include <vulkan/vulkan.hpp>

class Device;

class RenderPass {
public:
  RenderPass(const Device *device, VkFormat format);
  ~RenderPass();

  const VkRenderPass &getHandle() const { return mHandle; }

private:
  const Device *mDevice;
  VkRenderPass  mHandle;
};
