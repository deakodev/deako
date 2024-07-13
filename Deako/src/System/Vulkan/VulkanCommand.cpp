#include "VulkanCommand.h"
#include "dkpch.h"

#include "Deako/Core/Application.h"

#include "VulkanBase.h"
#include "VulkanFramebuffer.h"
#include "VulkanBuffer.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace Deako {

    std::vector<VkCommandBuffer> VulkanCommandPool::s_CommandBuffers;
    std::vector<VkCommandBuffer> VulkanCommandPool::s_ViewportCommandBuffers;

    // Command pools manage memory that is used to store buffers and command buffers are allocated from them
    void VulkanCommandPool::Create()
    {
        VulkanResources* vr = VulkanBase::GetResources();

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        // two possible flags for command pools:
        // • VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior)
        // • VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: allow command buffers to be rerecorded individually, without this flag they all have to be reset together
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        // Command buffers are executed by submitting them on one of the device queues, like the graphics and presentation queues. Each command pool can only allocate command buffers that are submitted on a single type of queue
        poolInfo.queueFamilyIndex = vr->graphicsFamily.value();

        VkResult result = vkCreateCommandPool(vr->device, &poolInfo, nullptr, &vr->commandPool);
        DK_CORE_ASSERT(!result);

        VulkanCommandPool::CreateBuffers(s_CommandBuffers);

        // Viewport
        VkCommandPoolCreateInfo viewportPoolInfo{};
        viewportPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        // two possible flags for command pools:
        // • VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior)
        // • VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: allow command buffers to be rerecorded individually, without this flag they all have to be reset together
        viewportPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        // Command buffers are executed by submitting them on one of the device queues, like the graphics and presentation queues. Each command pool can only allocate command buffers that are submitted on a single type of queue
        viewportPoolInfo.queueFamilyIndex = vr->graphicsFamily.value();

        result = vkCreateCommandPool(vr->device, &viewportPoolInfo, nullptr, &vr->viewportCommandPool);
        DK_CORE_ASSERT(!result);

        VulkanCommandPool::CreateBuffers(s_ViewportCommandBuffers);
    }

    void VulkanCommandPool::CleanUp()
    {
        VulkanResources* vr = VulkanBase::GetResources();

        vkDestroyCommandPool(vr->device, vr->commandPool, nullptr);
        vkDestroyCommandPool(vr->device, vr->viewportCommandPool, nullptr);
    }

    // Command buffers will be automatically freed when their command pool is destroyed
    void VulkanCommandPool::CreateBuffers(std::vector<VkCommandBuffer>& commandBuffers)
    {
        VulkanResources* vr = VulkanBase::GetResources();

        commandBuffers.resize(vr->imageCount);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = vr->commandPool;
        // levels:
        // • VK_COMMAND_BUFFER_LEVEL_PRIMARY: can be submitted to a queue for execution, but cannot be called from other command buffers
        // • VK_COMMAND_BUFFER_LEVEL_SECONDARY: cannot be submitted directly, but can be called from primary command buffers
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

        VkResult result = vkAllocateCommandBuffers(vr->device, &allocInfo, commandBuffers.data());
        DK_CORE_ASSERT(!result);
    }

    VkCommandBuffer VulkanCommandPool::Record(uint32_t currentFrame, uint32_t imageIndex)
    {
        VulkanResources* vr = VulkanBase::GetResources();

        VkCommandBuffer commandBuffer = s_CommandBuffers[currentFrame];

        vkResetCommandBuffer(commandBuffer, 0);

        // prepare recording to buffer
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        // flags:
        // • VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: command buffer will be rerecorded right after executing it once
        // • VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: this is a secondary command buffer that will be entirely within a single render pass
        // • VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: command buffer can be resubmitted while it is also already pending execution
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // required for imgui
        // relevant for secondary command buffers, specifies which state to inherit from the calling primary command buffers
        beginInfo.pInheritanceInfo = nullptr; // Optional

        // start recording to buffer
        VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
        DK_CORE_ASSERT(!result);

        // prepare render pass
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = vr->renderPass;
        // need to bind the framebuffer for the swapchain image we want to draw to, using the passed in imageIndex
        renderPassInfo.framebuffer = VulkanFramebufferPool::GetFramebuffer(imageIndex);
        // define the size of the render area
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = vr->imageExtent;
        // define the clear values to use for VK_ATTACHMENT_LOAD_OP_CLEAR(load operation for the color attachment)
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        // Note - all vkCmd functions are the ones that record to buffer

        // start render pass
        // final param:
        // • VK_SUBPASS_CONTENTS_INLINE: render pass commands will be embedded in primary command buffer itself, no secondary command buffers will be executed
        // • VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: render pass commands will be executed from secondary command buffers
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // bind the graphics pipeline
        // second param specifies if the pipeline object is a graphics or compute pipeline
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vr->graphicsPipeline);

        // const Ref<VertexBuffer>& vertexBuffer = VulkanBufferPool::GetVertexBuffer();
        // const Ref<IndexBuffer>& indexBuffer = VulkanBufferPool::GetIndexBuffer();

        // VkBuffer vertexBuffers[] = { vertexBuffer->GetBuffer() };
        // VkDeviceSize offsets[] = { 0 };
        // vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        // vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT16);

        // Note - in fixed functions section of pipeline, we specified viewport/scissor state to be dynamic. So we need to set them
        // VkViewport viewport{};
        // viewport.x = 0.0f;
        // viewport.y = 0.0f;
        // viewport.width = static_cast<float>(vr->imageExtent.width);
        // viewport.height = static_cast<float>(vr->imageExtent.height);
        // viewport.minDepth = 0.0f;
        // viewport.maxDepth = 1.0f;
        // vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        // VkRect2D scissor{};
        // scissor.offset = { 0, 0 };
        // scissor.extent = vr->imageExtent;
        // vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vr->pipelineLayout, 0, 1,
        //     &VulkanBufferPool::GetDescriptorSet(currentFrame), 0, nullptr);

        // // ***Draw*** command for the triangle
        // vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indexBuffer->GetIndices().size()), 1, 0, 0, 0);

        // ImGui
        LayerStack& layerStack = Application::Get().GetLayerStack();
        ImGuiLayer* imguiLayer = Application::Get().GetImGuiLayer();

        imguiLayer->Begin();
        for (Layer* layer : layerStack)
            layer->OnImGuiRender();
        imguiLayer->End(commandBuffer, vr->graphicsPipeline);

        // end the render pass
        vkCmdEndRenderPass(commandBuffer);

        result = vkEndCommandBuffer(commandBuffer);
        DK_CORE_ASSERT(!result);

        return commandBuffer;
    }

    VkCommandBuffer VulkanCommandPool::BeginSingleTimeCommands(VkCommandPool commandPool)
    {
        VulkanResources* vr = VulkanBase::GetResources();

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(vr->device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void VulkanCommandPool::EndSingleTimeCommands(VkCommandPool commandPool, VkCommandBuffer commandBuffer)
    {
        VulkanResources* vr = VulkanBase::GetResources();

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(vr->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

        vkQueueWaitIdle(vr->graphicsQueue);

        vkFreeCommandBuffers(vr->device, commandPool, 1, &commandBuffer);
    }

}
