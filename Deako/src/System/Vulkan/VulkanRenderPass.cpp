#include "VulkanRenderPass.h"
#include "dkpch.h"

#include "VulkanBase.h"

namespace Deako {

    // Tells Vulkan about framebuffer attachments that will be used while rendering
    // Specify how many color and depth buffers there will be, how many samples to use for each of them and how their contents should be handled throughout the rendering operations
    void VulkanRenderPass::Create()
    {
        VulkanResources* vr = VulkanBase::GetResources();

        // {//--- Color attachment --- ///
        //     VkAttachmentDescription colorAttachment{};
        //     colorAttachment.format = vr->imageFormat;
        //     colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // default 1, used for multisampling
        //     // loadOp:
        //     // • VK_ATTACHMENT_LOAD_OP_LOAD: preserve existing contents of attachment
        //     // • VK_ATTACHMENT_LOAD_OP_CLEAR: clear values to a constant at the start
        //     // • VK_ATTACHMENT_LOAD_OP_DONT_CARE: existing contents are undefined, we don’t care about them
        //     // storeOp:
        //     // • VK_ATTACHMENT_STORE_OP_STORE: rendered contents will be stored in memory and can be read later
        //     // • VK_ATTACHMENT_STORE_OP_DONT_CARE: contents of framebuffer will be undefined after rendering operation
        //     colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // clear fb to black before drawing a new frame
        //     colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        //     // Our app won’t do anything with stencil buffer, so results of loading and storing are irrelevant
        //     colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        //     colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        //     // layout of the pixels in memory can change based on what you’re trying to do with an image:
        //     // • VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: images used as color attachment
        //     // • VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: images to be presented in swap chain
        //     // • VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: images to be used as destination for a memory copy operation
        //     colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        //     colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        //     //--- Subpasses and attachment references --- ///
        //     VkAttachmentReference colorAttachmentRef{};
        //     colorAttachmentRef.attachment = 0; // one subpass for now so index 0
        //     colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        //     VkSubpassDescription subpass{};
        //     subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // Explicit, graphics subpass
        //     subpass.colorAttachmentCount = 1;
        //     // Specify the reference to the color attachment, the index of attachment is directly referenced from the fragment shader with the layout(location = 0)out vec4 outColor directive!
        //     subpass.pColorAttachments = &colorAttachmentRef;

        //     // the following other types of attachments can be referenced by a subpass:
        //     // • pInputAttachments: Attachments that are read from a shader
        //     // • pResolveAttachments: Attachments used for multisampling color attachments
        //     // • pDepthStencilAttachment: Attachment for depth and stencil data
        //     // • pPreserveAttachments: Attachments that are not used by this subpass, but for which the data must be preserved

        //     VkSubpassDependency dependency{};
        //     dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        //     dependency.dstSubpass = 0;
        //     dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        //     dependency.srcAccessMask = 0;
        //     dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        //     dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        //     VkRenderPassCreateInfo renderPassInfo{};
        //     renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        //     renderPassInfo.attachmentCount = 1;
        //     renderPassInfo.pAttachments = &colorAttachment;
        //     renderPassInfo.subpassCount = 1;
        //     renderPassInfo.pSubpasses = &subpass;
        //     renderPassInfo.dependencyCount = 1;
        //     renderPassInfo.pDependencies = &dependency;

        //     VkResult result = vkCreateRenderPass(vr->device, &renderPassInfo, nullptr, &vr->renderPass);
        //     DK_CORE_ASSERT(!result);
        // }

        {
            std::array<VkAttachmentDescription, 2> attachments = {};
            // Color attachment
            attachments[0].format = vr->imageFormat;
            attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            // Depth attachment
            attachments[1].format = DepthAttachment::FindDepthFormat();
            attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentReference colorReference = {};
            colorReference.attachment = 0;
            colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkAttachmentReference depthReference = {};
            depthReference.attachment = 1;
            depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpassDescription = {};
            subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDescription.colorAttachmentCount = 1;
            subpassDescription.pColorAttachments = &colorReference;
            subpassDescription.pDepthStencilAttachment = &depthReference;
            subpassDescription.inputAttachmentCount = 0;
            subpassDescription.pInputAttachments = nullptr;
            subpassDescription.preserveAttachmentCount = 0;
            subpassDescription.pPreserveAttachments = nullptr;
            subpassDescription.pResolveAttachments = nullptr;

            // Subpass dependencies for layout transitions
            std::array<VkSubpassDependency, 2> dependencies;

            dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass = 0;
            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            dependencies[1].srcSubpass = 0;
            dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            VkRenderPassCreateInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            renderPassInfo.pAttachments = attachments.data();
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpassDescription;
            renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
            renderPassInfo.pDependencies = dependencies.data();

            VkResult result = vkCreateRenderPass(vr->device, &renderPassInfo, nullptr, &vr->renderPass);
            DK_CORE_ASSERT(!result);
        }

        {
            std::array<VkAttachmentDescription, 2> attachments = {};
            // Color attachment
            attachments[0].format = vr->imageFormat;
            attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            // Depth attachment
            attachments[1].format = DepthAttachment::FindDepthFormat();
            attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentReference colorReference = {};
            colorReference.attachment = 0;
            colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkAttachmentReference depthReference = {};
            depthReference.attachment = 1;
            depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpassDescription = {};
            subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDescription.colorAttachmentCount = 1;
            subpassDescription.pColorAttachments = &colorReference;
            subpassDescription.pDepthStencilAttachment = &depthReference;
            subpassDescription.inputAttachmentCount = 0;
            subpassDescription.pInputAttachments = nullptr;
            subpassDescription.preserveAttachmentCount = 0;
            subpassDescription.pPreserveAttachments = nullptr;
            subpassDescription.pResolveAttachments = nullptr;

            // Subpass dependencies for layout transitions
            std::array<VkSubpassDependency, 2> dependencies;

            dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass = 0;
            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            dependencies[1].srcSubpass = 0;
            dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            VkRenderPassCreateInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            renderPassInfo.pAttachments = attachments.data();
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpassDescription;
            renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
            renderPassInfo.pDependencies = dependencies.data();

            VkResult result = vkCreateRenderPass(vr->device, &renderPassInfo, nullptr, &vr->viewportRenderPass);
            DK_CORE_ASSERT(!result);
        }

    }

    void VulkanRenderPass::CleanUp()
    {
        VulkanResources* vr = VulkanBase::GetResources();

        vkDestroyRenderPass(vr->device, vr->viewportRenderPass, nullptr);
        vkDestroyRenderPass(vr->device, vr->renderPass, nullptr);
    }

}
