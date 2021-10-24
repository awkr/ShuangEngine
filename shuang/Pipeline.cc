#include "Pipeline.h"
#include "Device.h"
#include "FileSystem.h"
#include "Logger.h"
#include "Macros.h"
#include "RenderPass.h"

Pipeline::Pipeline(const Device *device, const RenderPass *renderPass)
    : mDevice{device} {
  // Create a blank pipeline layout.
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
  ASSERT(vkCreatePipelineLayout(device->getHandle(), &pipelineLayoutCreateInfo,
                                nullptr, &mPipelineLayout));

  VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

  // Specify we will use triangle lists to draw geometry.
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
  inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  // Specify rasterization state.
  VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
  rasterizationStateCreateInfo.cullMode  = VK_CULL_MODE_BACK_BIT;
  rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizationStateCreateInfo.lineWidth = 1.0f;

  // Our attachment will write to all color channels, but no blending is
  // enabled.
  VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
  colorBlendAttachmentState.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
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

  // Specify that these states will be dynamicStateCreateInfo, i.e. not part of
  // pipeline state object.
  std::array<VkDynamicState, 2> dynamicStates{VK_DYNAMIC_STATE_VIEWPORT,
                                              VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
  dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();
  dynamicStateCreateInfo.dynamicStateCount =
      static_cast<uint32_t>(dynamicStates.size());

  // Load our SPIR-V shaders.
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

  VkGraphicsPipelineCreateInfo pipelineCreateInfo{
      VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
  pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
  pipelineCreateInfo.pStages    = shaderStages.data();
  pipelineCreateInfo.pVertexInputState   = &vertexInputStateCreateInfo;
  pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
  pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
  pipelineCreateInfo.pColorBlendState    = &colorBlendStateCreateInfo;
  pipelineCreateInfo.pMultisampleState   = &multisampleStateCreateInfo;
  pipelineCreateInfo.pViewportState      = &viewportStateCreateInfo;
  pipelineCreateInfo.pDepthStencilState  = &depthStencilStateCreateInfo;
  pipelineCreateInfo.pDynamicState       = &dynamicStateCreateInfo;

  // We need to specify the pipeline layout and the render pass description up
  // front as well.
  pipelineCreateInfo.renderPass = renderPass->getHandle();
  pipelineCreateInfo.layout     = mPipelineLayout;

  ASSERT(vkCreateGraphicsPipelines(device->getHandle(), VK_NULL_HANDLE, 1,
                                   &pipelineCreateInfo, nullptr, &mHandle));

  // Pipeline is baked, we can delete the shader modules now.
  vkDestroyShaderModule(device->getHandle(), shaderStages[0].module, nullptr);
  vkDestroyShaderModule(device->getHandle(), shaderStages[1].module, nullptr);
}

Pipeline::~Pipeline() {
  logInfo(__func__);
  vkDestroyPipeline(mDevice->getHandle(), mHandle, nullptr);
  vkDestroyPipelineLayout(mDevice->getHandle(), mPipelineLayout, nullptr);
}

VkShaderModule Pipeline::createShaderModule(const char *path) {
  auto source = filesystem::read(path);

  VkShaderModuleCreateInfo shaderModuleCreateInfo{
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
  shaderModuleCreateInfo.codeSize = source.size();
  shaderModuleCreateInfo.pCode =
      reinterpret_cast<const uint32_t *>(source.data());

  VkShaderModule shaderModule;
  ASSERT(vkCreateShaderModule(mDevice->getHandle(), &shaderModuleCreateInfo,
                              nullptr, &shaderModule));
  return shaderModule;
}
