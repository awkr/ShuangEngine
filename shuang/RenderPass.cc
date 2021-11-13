#include "RenderPass.h"
#include "Device.h"
#include "Logger.h"
#include "Macros.h"

RenderPass::RenderPass(const std::shared_ptr<Device> &device, VkFormat imageFormat,
                       VkFormat depthFormat)
    : mDevice{device} {
  std::array<VkAttachmentDescription, 2> attachments{};
  // Color attachment
  attachments[0].format  = imageFormat;
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  // When starting the frame, we want tiles to be cleared.
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  // When ending the frame, we want tiles to be written out.
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  // Don't care about stencil since we're not using it.
  attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  // The image layout will be undefined when the render pass begins.
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  // After the render pass is complete, we will transition to PRESENT_SRC_KHR layout.
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  // Depth attachment
  attachments[1].format         = depthFormat;
  attachments[1].samples        = VK_SAMPLE_COUNT_1_BIT;
  attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[1].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // We have one subpass. This subpass has one color attachment and one depth attachment.

  VkAttachmentReference colorRef{};
  colorRef.attachment = 0;
  colorRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthRef{};
  depthRef.attachment = 1;
  depthRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // We will end up with two transitions.
  // The first one happens right before we start subpass #0, where UNDEFINED is transitioned into
  // COLOR_ATTACHMENT_OPTIMAL. The final layout in the render pass attachment states
  // PRESENT_SRC_KHR, so we will get a final transition from COLOR_ATTACHMENT_OPTIMAL to
  // PRESENT_SRC_KHR.
  VkSubpassDescription subpassDescription    = {0};
  subpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDescription.colorAttachmentCount    = 1;
  subpassDescription.pColorAttachments       = &colorRef;
  subpassDescription.pDepthStencilAttachment = &depthRef;
  subpassDescription.inputAttachmentCount    = 0;
  subpassDescription.pInputAttachments       = nullptr;
  subpassDescription.preserveAttachmentCount = 0;
  subpassDescription.pPreserveAttachments    = nullptr;
  subpassDescription.pResolveAttachments     = nullptr;

  // Create a dependency to external events.
  // We need to wait for the WSI semaphore to signal.
  // Only pipeline stages which depend on COLOR_ATTACHMENT_OUTPUT_BIT will actually wait for the
  // semaphore, so we must also wait for that pipeline stage.

  // Subpass dependencies for layout transitions
  std::array<VkSubpassDependency, 2> dependencies{};

  dependencies[0].srcSubpass    = VK_SUBPASS_EXTERNAL;
  dependencies[0].dstSubpass    = 0;
  dependencies[0].srcStageMask  = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[0].dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  // Since we changed the image layout, we need to make the memory visible to color attachment to
  // modify.
  dependencies[0].dstAccessMask =
      VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  dependencies[1].srcSubpass   = 0;
  dependencies[1].dstSubpass   = VK_SUBPASS_EXTERNAL;
  dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[1].srcAccessMask =
      VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[1].dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  // Finally, create the render pass.
  VkRenderPassCreateInfo renderPassCreateInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
  renderPassCreateInfo.attachmentCount        = attachments.size();
  renderPassCreateInfo.pAttachments           = attachments.data();
  renderPassCreateInfo.subpassCount           = 1;
  renderPassCreateInfo.pSubpasses             = &subpassDescription;
  renderPassCreateInfo.dependencyCount        = dependencies.size();
  renderPassCreateInfo.pDependencies          = dependencies.data();

  vkOK(vkCreateRenderPass(device->getHandle(), &renderPassCreateInfo, nullptr, &mHandle));
}

RenderPass::~RenderPass() {
  log_func;
  vkDestroyRenderPass(mDevice->getHandle(), mHandle, nullptr);
}
