#include "VulkanRenderPass.h"
#include "dkpch.h"

#include "VulkanDevice.h"
#include "VulkanSwapChain.h"

namespace Deako {

    VkRenderPass VulkanRenderPass::s_RenderPass;

    // Tells Vulkan about framebuffer attachments that will be used while rendering
    // Specify how many color and depth buffers there will be, how many samples to use for each of them and how their contents should be handled throughout the rendering operations
    void VulkanRenderPass::Create()
    {
        VkDevice device = VulkanDevice::GetLogical();
        VkFormat swapChainImageFormat = VulkanSwapChain::GetImageFormat();

        //--- Color attachment --- ///
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // default 1, used for multisampling
        // loadOp:
        // • VK_ATTACHMENT_LOAD_OP_LOAD: preserve existing contents of attachment
        // • VK_ATTACHMENT_LOAD_OP_CLEAR: clear values to a constant at the start
        // • VK_ATTACHMENT_LOAD_OP_DONT_CARE: existing contents are undefined, we don’t care about them
        // storeOp:
        // • VK_ATTACHMENT_STORE_OP_STORE: rendered contents will be stored in memory and can be read later
        // • VK_ATTACHMENT_STORE_OP_DONT_CARE: contents of framebuffer will be undefined after rendering operation
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // clear fb to black before drawing a new frame
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        // Our app won’t do anything with stencil buffer, so results of loading and storing are irrelevant
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        // layout of the pixels in memory can change based on what you’re trying to do with an image:
        // • VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: images used as color attachment
        // • VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: images to be presented in swap chain
        // • VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: images to be used as destination for a memory copy operation
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        //--- Subpasses and attachment references --- ///
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0; // one subpass for now so index 0
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // Explicit, graphics subpass
        subpass.colorAttachmentCount = 1;
        // Specify the reference to the color attachment, the index of attachment is directly referenced from the fragment shader with the layout(location = 0)out vec4 outColor directive!
        subpass.pColorAttachments = &colorAttachmentRef;

        // the following other types of attachments can be referenced by a subpass:
        // • pInputAttachments: Attachments that are read from a shader
        // • pResolveAttachments: Attachments used for multisampling color attachments
        // • pDepthStencilAttachment: Attachment for depth and stencil data
        // • pPreserveAttachments: Attachments that are not used by this subpass, but for which the data must be preserved

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        VkResult result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &s_RenderPass);
        DK_CORE_ASSERT(!result, "Failed to create render pass!");
    }

    void VulkanRenderPass::CleanUp()
    {
        VkDevice device = VulkanDevice::GetLogical();
        vkDestroyRenderPass(device, s_RenderPass, nullptr);
    }

}
