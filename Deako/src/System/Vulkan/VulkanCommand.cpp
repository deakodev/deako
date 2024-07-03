#include "VulkanCommand.h"

#include "VulkanDevice.h"
#include "VulkanRenderPass.h"
#include "VulkanFramebuffer.h"
#include "VulkanSwapChain.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"

namespace Deako {

    const int MAX_FRAMES_IN_FLIGHT = 2;

    VkCommandPool VulkanCommandPool::s_CommandPool;
    std::vector<VkCommandBuffer> VulkanCommandPool::s_CommandBuffers;

    // Command pools manage memory that is used to store buffers and command buffers are allocated from them
    void VulkanCommandPool::Create()
    {
        VkDevice device = VulkanDevice::GetLogical();
        QueueFamilyIndices indices = VulkanDevice::GetQueueFamilyIndices();

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        // two possible flags for command pools:
        // • VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior)
        // • VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: allow command buffers to be rerecorded individually, without this flag they all have to be reset together
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        // Command buffers are executed by submitting them on one of the device queues, like the graphics and presentation queues. Each command pool can only allocate command buffers that are submitted on a single type of queue
        poolInfo.queueFamilyIndex = indices.graphicsFamily.value();

        VkResult result = vkCreateCommandPool(device, &poolInfo, nullptr, &s_CommandPool);
        DK_CORE_ASSERT(!result, "Failed to create command pool!");

        VulkanCommandPool::CreateBuffers();
    }

    void VulkanCommandPool::CleanUp()
    {
        VkDevice device = VulkanDevice::GetLogical();
        vkDestroyCommandPool(device, s_CommandPool, nullptr);
    }

    // Command buffers will be automatically freed when their command pool is destroyed
    void VulkanCommandPool::CreateBuffers()
    {
        VkDevice device = VulkanDevice::GetLogical();

        s_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = VulkanCommandPool::s_CommandPool;
        // levels:
        // • VK_COMMAND_BUFFER_LEVEL_PRIMARY: can be submitted to a queue for execution, but cannot be called from other command buffers
        // • VK_COMMAND_BUFFER_LEVEL_SECONDARY: cannot be submitted directly, but can be called from primary command buffers
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)s_CommandBuffers.size();

        VkResult result = vkAllocateCommandBuffers(device, &allocInfo, s_CommandBuffers.data());
        DK_CORE_ASSERT(!result, "Failed to allocate command buffers!");
    }

    VkCommandBuffer VulkanCommandPool::Record(uint32_t currentFrame, uint32_t imageIndex)
    {
        VkCommandBuffer commandBuffer = s_CommandBuffers[currentFrame];

        vkResetCommandBuffer(commandBuffer, 0);

        // prepare recording to buffer
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        // flags:
        // • VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: command buffer will be rerecorded right after executing it once
        // • VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: this is a secondary command buffer that will be entirely within a single render pass
        // • VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: command buffer can be resubmitted while it is also already pending execution
        beginInfo.flags = 0; // Optional
        // relevant for secondary command buffers, specifies which state to inherit from the calling primary command buffers
        beginInfo.pInheritanceInfo = nullptr; // Optional

        // start recording to buffer
        VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
        DK_CORE_ASSERT(!result, "Failed to begin recording command buffer!");

        // prepare render pass
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = VulkanRenderPass::GetRenderPass();
        // need to bind the framebuffer for the swapchain image we want to draw to, using the passed in imageIndex
        const auto& swapChainFramebuffers = VulkanFramebufferPool::GetFramebuffers();
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        // define the size of the render area
        renderPassInfo.renderArea.offset = { 0, 0 };
        const auto& swapChainExtent = VulkanSwapChain::GetExtent();
        renderPassInfo.renderArea.extent = swapChainExtent;
        // define the clear values to use for VK_ATTACHMENT_LOAD_OP_CLEAR(load operation for the color attachment)
        VkClearValue clearColor = { {{ 0.0f, 0.0f, 0.0f, 1.0f }} };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        // Note - all vkCmd functions are the ones that record to buffer

        // start render pass
        // final param:
        // • VK_SUBPASS_CONTENTS_INLINE: render pass commands will be embedded in primary command buffer itself, no secondary command buffers will be executed
        // • VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: render pass commands will be executed from secondary command buffers
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // bind the graphics pipeline
        // second param specifies if the pipeline object is a graphics or compute pipeline
        VkPipeline graphicsPipeline = VulkanPipeline::GetPipeline();
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        const Ref<VertexBuffer>& vertexBuffer = VulkanBufferPool::GetVertexBuffer();
        const Ref<IndexBuffer>& indexBuffer = VulkanBufferPool::GetIndexBuffer();

        VkBuffer vertexBuffers[] = { vertexBuffer->GetBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT16);

        // Note - in fixed functions section of pipeline, we specified viewport/scissor state to be dynamic. So we need to set them
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChainExtent.width);
        viewport.height = static_cast<float>(swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkPipelineLayout pipelineLayout = VulkanPipeline::GetPipelineLayout();
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
            &VulkanBufferPool::GetDescriptorSet(currentFrame), 0, nullptr);

        // ***Draw*** command for the triangle
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indexBuffer->GetIndices().size()), 1, 0, 0, 0);

        // end the render pass
        vkCmdEndRenderPass(commandBuffer);

        result = vkEndCommandBuffer(commandBuffer);
        DK_CORE_ASSERT(!result, "Failed to record command buffer!");

        return commandBuffer;
    }

}
