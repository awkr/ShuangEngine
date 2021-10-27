#include "DescriptorSetLayout.h"
#include "Device.h"
#include "Logger.h"
#include "Macros.h"

DescriptorSetLayout::DescriptorSetLayout(const std::shared_ptr<Device> &device) : mDevice{device} {
  std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings = {
      // bind point 0: vertex shader uniform buffer
      createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                       VK_SHADER_STAGE_VERTEX_BIT, 0),
      // bind point 1: fragment shader combined sampler
      createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                       VK_SHADER_STAGE_FRAGMENT_BIT, 1),
  };

  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
  descriptorSetLayoutCreateInfo.sType     = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();
  descriptorSetLayoutCreateInfo.bindingCount =
      static_cast<uint32_t>(descriptorSetLayoutBindings.size());

  vkAssert(vkCreateDescriptorSetLayout(device->getHandle(), &descriptorSetLayoutCreateInfo, nullptr,
                                       &mHandle));
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
