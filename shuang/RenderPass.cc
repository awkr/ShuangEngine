#include "RenderPass.h"
#include "Device.h"
#include "Logger.h"
#include "Macros.h"

RenderPass::RenderPass(const Device *device, const VkFormat format)
    : mDevice{device} {
  VkAttachmentDescription attachment = {0};
  attachment.format                  = format;
  attachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
  // When starting the frame, we want tiles to be cleared.
  attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  // When ending the frame, we want tiles to be written out.
  attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  // Don't care about stencil since we're not using it.
  attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  // The image layout will be undefined when the render pass begins.
  attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  // After the render pass is complete, we will transition to PRESENT_SRC_KHR
  // layout.
  attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  // We have one subpass. This subpass has one color attachment.
  // While executing this subpass, the attachment will be in attachment optimal
  // layout.
  VkAttachmentReference attachmentRef = {
      0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

  // We will end up with two transitions.
  // The first one happens right before we start subpass #0, where
  // UNDEFINED is transitioned into COLOR_ATTACHMENT_OPTIMAL.
  // The final layout in the render pass attachment states PRESENT_SRC_KHR, so
  // we will get a final transition from COLOR_ATTACHMENT_OPTIMAL to
  // PRESENT_SRC_KHR.
  VkSubpassDescription subpass = {0};
  subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments    = &attachmentRef;

  // Create a dependency to external events.
  // We need to wait for the WSI semaphore to signal.
  // Only pipeline stages which depend on COLOR_ATTACHMENT_OUTPUT_BIT will
  // actually wait for the semaphore, so we must also wait for that pipeline
  // stage.
  VkSubpassDependency dependency = {0};
  dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass          = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  // Since we changed the image layout, we need to make the memory visible to
  // color attachment to modify.
  dependency.srcAccessMask = 0;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  // Finally, create the render pass.
  VkRenderPassCreateInfo renderPassCreateInfo = {
      VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
  renderPassCreateInfo.attachmentCount = 1;
  renderPassCreateInfo.pAttachments    = &attachment;
  renderPassCreateInfo.subpassCount    = 1;
  renderPassCreateInfo.pSubpasses      = &subpass;
  renderPassCreateInfo.dependencyCount = 1;
  renderPassCreateInfo.pDependencies   = &dependency;

  ASSERT(vkCreateRenderPass(device->getHandle(), &renderPassCreateInfo, nullptr,
                            &mHandle));
}

RenderPass::~RenderPass() {
  logInfo(__func__);
  vkDestroyRenderPass(mDevice->getHandle(), mHandle, nullptr);
}
