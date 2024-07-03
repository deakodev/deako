#include "VulkanFramebuffer.h"

#include "VulkanDevice.h"
#include "VulkanRenderPass.h"
#include "VulkanSwapChain.h"

namespace Deako {

    std::vector<VkFramebuffer> VulkanFramebufferPool::s_Framebuffers;

    void VulkanFramebufferPool::Create()
    {
        VkDevice device = VulkanDevice::GetLogical();

        // First resize the list to fit all of the image views
        auto& swapChainImageViews = VulkanSwapChain::GetImageViews();
        s_Framebuffers.resize(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); i++)
        {
            VkImageView attachments[] = { swapChainImageViews[i] };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            // can only use a framebuffer with render passes that are compatible, Eg. they use the same number and type of attachments
            framebufferInfo.renderPass = VulkanRenderPass::GetRenderPass();
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            const auto& swapChainExtent = VulkanSwapChain::GetExtent();
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            VkResult result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &s_Framebuffers[i]);
            DK_CORE_ASSERT(!result, "Failed to create framebuffer!");
        }
    }

    // TODO: refering to use in VulkanSwapChain::Recreate(), pg. 140, may need to separate *swap chain* framebuffer portion if adding different types of framebuffers so we dont accidently CleanUp the wrong framebuffers
    void VulkanFramebufferPool::CleanUp()
    {
        VkDevice device = VulkanDevice::GetLogical();
        for (auto framebuffer : s_Framebuffers)
            vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

}
