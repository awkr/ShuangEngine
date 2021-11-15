#include "DescriptorSetLayout.h"
#include "Device.h"
#include "Log.h"
#include "Macros.h"

DescriptorSetLayout::DescriptorSetLayout(const std::shared_ptr<Device> &device) : mDevice{device} {
  std::vector<VkDescriptorSetLayoutBinding> bindings = {
      createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                       VK_SHADER_STAGE_VERTEX_BIT, 0),
  };

  VkDescriptorSetLayoutCreateInfo createInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
  createInfo.pBindings    = bindings.data();
  createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  vkOK(vkCreateDescriptorSetLayout(device->getHandle(), &createInfo, nullptr, &mHandle));
}

DescriptorSetLayout::~DescriptorSetLayout() {
  log_func;
  vkDestroyDescriptorSetLayout(mDevice->getHandle(), mHandle, nullptr);
}

VkDescriptorSetLayoutBinding DescriptorSetLayout::createDescriptorSetLayoutBinding(
    VkDescriptorType type, VkShaderStageFlags stages, uint32_t binding, uint32_t count) {
  VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{};
  descriptorSetLayoutBinding.descriptorType  = type;
  descriptorSetLayoutBinding.stageFlags      = stages;
  descriptorSetLayoutBinding.binding         = binding;
  descriptorSetLayoutBinding.descriptorCount = count;
  return descriptorSetLayoutBinding;
}
