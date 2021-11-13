#include "DescriptorSet.h"
#include "DescriptorPool.h"
#include "Device.h"
#include "Logger.h"
#include "Macros.h"

DescriptorSet::DescriptorSet(const std::shared_ptr<Device>            &device,
                             const std::shared_ptr<DescriptorPool>    &pool,
                             uint32_t                                  descriptorSetCount,
                             const std::vector<VkDescriptorSetLayout> &descriptorSetLayouts,
                             const VkDescriptorBufferInfo             &bufferInfo) {
  VkDescriptorSetAllocateInfo allocateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  allocateInfo.descriptorPool     = pool->getHandle();
  allocateInfo.descriptorSetCount = descriptorSetCount;
  allocateInfo.pSetLayouts        = descriptorSetLayouts.data();

  vkOK(vkAllocateDescriptorSets(device->getHandle(), &allocateInfo, &mHandle));

  std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
      // Binding 0 : vertex shader uniform buffer
      createWriteDescriptorSet(mHandle, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &bufferInfo),
  };
  vkUpdateDescriptorSets(device->getHandle(), writeDescriptorSets.size(),
                         writeDescriptorSets.data(), 0, nullptr);
}

VkWriteDescriptorSet DescriptorSet::createWriteDescriptorSet(
    VkDescriptorSet dstSet, VkDescriptorType type, uint32_t sdtBinding,
    const VkDescriptorBufferInfo *bufferInfo, uint32_t descriptorCount) {
  VkWriteDescriptorSet writeDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  writeDescriptorSet.dstSet          = dstSet;
  writeDescriptorSet.descriptorType  = type;
  writeDescriptorSet.dstBinding      = sdtBinding;
  writeDescriptorSet.pBufferInfo     = bufferInfo;
  writeDescriptorSet.descriptorCount = descriptorCount;
  return writeDescriptorSet;
}
