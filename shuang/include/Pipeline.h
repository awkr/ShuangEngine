#pragma once

#include <vulkan/vulkan.hpp>

class Device;
class RenderPass;
class DescriptorSetLayout;

class Pipeline {
public:
  Pipeline(const std::shared_ptr<Device> &device, const std::shared_ptr<RenderPass> &renderPass,
           const std::vector<std::shared_ptr<DescriptorSetLayout>> &descriptorSetLayouts = {});
  ~Pipeline();

  const VkPipeline &getHandle() const { return mHandle; }

private:
  VkShaderModule createShaderModule(const char *path);
  static VkVertexInputBindingDescription
  createVertexInputBindingDescription(uint32_t binding, uint32_t stride,
                                      VkVertexInputRate inputRate);
  static VkVertexInputAttributeDescription createVertexInputAttributeDescription(uint32_t binding,
                                                                                 uint32_t location,
                                                                                 VkFormat format,
                                                                                 uint32_t offset);

  const std::shared_ptr<Device> &mDevice = nullptr;
  VkPipelineLayout               mPipelineLayout;
  VkPipeline                     mHandle;
};
