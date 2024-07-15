#include "VulkanFramebuffer.h"
#include "dkpch.h"

#include "VulkanSwapChain.h"

namespace Deako {

    Ref<VulkanResources> FramebufferPool::s_VR = VulkanBase::GetResources();

    void FramebufferPool::CreateFramebuffers()
    {
        s_VR->framebuffers.resize(s_VR->swapChainImageViews.size());
        for (size_t i = 0; i < s_VR->framebuffers.size(); i++)
        {
            std::array<VkImageView, 2> attachments = { s_VR->swapChainImageViews[i], s_VR->depthImageView };

            VkFramebufferCreateInfo framebufferInfo = VulkanInitializers::FramebufferCreateInfo();
            // can only use a framebuffer with render passes that are compatible, Eg. they use the same number and type of attachments
            framebufferInfo.renderPass = s_VR->renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = s_VR->imageExtent.width;
            framebufferInfo.height = s_VR->imageExtent.height;
            framebufferInfo.layers = 1;

            VkResult result = vkCreateFramebuffer(s_VR->device, &framebufferInfo, nullptr, &s_VR->framebuffers[i]);
            DK_CORE_ASSERT(!result);
        }

        // Viewport
        s_VR->viewportFramebuffers.resize(s_VR->viewportImageViews.size());
        for (size_t i = 0; i < s_VR->viewportFramebuffers.size(); i++)
        {

            std::array<VkImageView, 2> attachments = { s_VR->viewportImageViews[i], s_VR->depthImageView };

            VkFramebufferCreateInfo framebufferInfo = VulkanInitializers::FramebufferCreateInfo();
            framebufferInfo.renderPass = s_VR->viewportRenderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = s_VR->imageExtent.width;
            framebufferInfo.height = s_VR->imageExtent.height;
            framebufferInfo.layers = 1;

            VkResult result = vkCreateFramebuffer(s_VR->device, &framebufferInfo, nullptr, &s_VR->viewportFramebuffers[i]);
            DK_CORE_ASSERT(!result);
        }
    }

    void FramebufferPool::CleanUp()
    {
        for (auto framebuffer : s_VR->framebuffers)
            vkDestroyFramebuffer(s_VR->device, framebuffer, nullptr);

        for (auto framebuffer : s_VR->viewportFramebuffers)
            vkDestroyFramebuffer(s_VR->device, framebuffer, nullptr);
    }

}
