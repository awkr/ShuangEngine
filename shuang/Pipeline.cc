#include "Pipeline.h"
#include "Device.h"
#include "FileSystem.h"
#include "Logger.h"
#include "Macros.h"
#include "RenderPass.h"
#include "Vertex.h"

Pipeline::Pipeline(const std::shared_ptr<Device>            &device,
                   const std::shared_ptr<RenderPass>        &renderPass,
                   const std::vector<VkDescriptorSetLayout> &descriptorSetLayouts)
    : mDevice{device} {
  // Create a pipeline layout.
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
  pipelineLayoutCreateInfo.pSetLayouts    = descriptorSetLayouts.data();
  pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayouts.size();
  vkAssert(
      vkCreatePipelineLayout(mDevice->getHandle(), &pipelineLayoutCreateInfo, nullptr, &mLayout));

  // Vertex binding and attributes
  // Binding descriptions
  std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
      createVertexInputBindingDescription(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX),
  };
  // Attribute descriptions
  std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
      createVertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT,
                                            offsetof(Vertex, position)),
      createVertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32B32_SFLOAT,
                                            offsetof(Vertex, color)),
  };

  VkPipelineVertexInputStateCreateInfo vertexInputState{
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
  vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
  vertexInputState.vertexBindingDescriptionCount =
      static_cast<uint32_t>(vertexInputBindings.size());
  vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();
  vertexInputState.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(vertexInputAttributes.size());

  // Specify we will use triangle lists to draw geometry.
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
  inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  // Specify rasterization state.
  VkPipelineRasterizationStateCreateInfo rasterizationState{
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
  //  rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizationState.cullMode = VK_CULL_MODE_NONE;
  //  rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizationState.lineWidth = 1.0f;

  // Our attachment will write to all color channels, but no blending is enabled.
  VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
  colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                             VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
  colorBlendStateCreateInfo.attachmentCount = 1;
  colorBlendStateCreateInfo.pAttachments    = &colorBlendAttachmentState;

  // We will have one viewportStateCreateInfo and scissor box.
  VkPipelineViewportStateCreateInfo viewportStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
  viewportStateCreateInfo.viewportCount = 1;
  viewportStateCreateInfo.scissorCount  = 1;

  // Disable all depth testing.
  VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

  // No multisampling.
  VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
  multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  // Specify that these states will be dynamicStateCreateInfo, i.e. not part of pipeline state
  // object.
  std::array<VkDynamicState, 2> dynamicStates{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
  dynamicStateCreateInfo.pDynamicStates    = dynamicStates.data();
  dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());

  // Load SPIR-V shaders.
  std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

  // Vertex stage of the pipeline
  shaderStages[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
  shaderStages[0].module = createShaderModule("shaders/triangle.vert.spv");
  shaderStages[0].pName  = "main";

  // Fragment stage of the pipeline
  shaderStages[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
  shaderStages[1].module = createShaderModule("shaders/triangle.frag.spv");
  shaderStages[1].pName  = "main";

  VkGraphicsPipelineCreateInfo pipelineCreateInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
  pipelineCreateInfo.stageCount          = static_cast<uint32_t>(shaderStages.size());
  pipelineCreateInfo.pStages             = shaderStages.data();
  pipelineCreateInfo.pVertexInputState   = &vertexInputState;
  pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
  pipelineCreateInfo.pRasterizationState = &rasterizationState;
  pipelineCreateInfo.pColorBlendState    = &colorBlendStateCreateInfo;
  pipelineCreateInfo.pMultisampleState   = &multisampleStateCreateInfo;
  pipelineCreateInfo.pViewportState      = &viewportStateCreateInfo;
  pipelineCreateInfo.pDepthStencilState  = &depthStencilStateCreateInfo;
  pipelineCreateInfo.pDynamicState       = &dynamicStateCreateInfo;

  // We need to specify the pipeline layout and the render pass description up front as well.
  pipelineCreateInfo.renderPass = renderPass->getHandle();
  pipelineCreateInfo.layout     = mLayout;

  vkAssert(vkCreateGraphicsPipelines(device->getHandle(), VK_NULL_HANDLE, 1, &pipelineCreateInfo,
                                     nullptr, &mHandle));

  // Pipeline is baked, we can delete the shader modules now.
  vkDestroyShaderModule(device->getHandle(), shaderStages[0].module, nullptr);
  vkDestroyShaderModule(device->getHandle(), shaderStages[1].module, nullptr);
}

Pipeline::~Pipeline() {
  log_func;
  vkDestroyPipeline(mDevice->getHandle(), mHandle, nullptr);
  vkDestroyPipelineLayout(mDevice->getHandle(), mLayout, nullptr);
}

VkShaderModule Pipeline::createShaderModule(const char *path) {
  auto source = filesystem::read(path);

  VkShaderModuleCreateInfo shaderModuleCreateInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
  shaderModuleCreateInfo.codeSize = source.size();
  shaderModuleCreateInfo.pCode    = reinterpret_cast<const uint32_t *>(source.data());

  VkShaderModule shaderModule;
  vkAssert(
      vkCreateShaderModule(mDevice->getHandle(), &shaderModuleCreateInfo, nullptr, &shaderModule));
  return shaderModule;
}

VkVertexInputBindingDescription
Pipeline::createVertexInputBindingDescription(uint32_t binding, uint32_t stride,
                                              VkVertexInputRate inputRate) {
  VkVertexInputBindingDescription description{};
  description.binding   = binding;
  description.stride    = stride;
  description.inputRate = inputRate;
  return description;
}

VkVertexInputAttributeDescription Pipeline::createVertexInputAttributeDescription(uint32_t binding,
                                                                                  uint32_t location,
                                                                                  VkFormat format,
                                                                                  uint32_t offset) {
  VkVertexInputAttributeDescription description{};
  description.location = location;
  description.binding  = binding;
  description.format   = format;
  description.offset   = offset;
  return description;
}
