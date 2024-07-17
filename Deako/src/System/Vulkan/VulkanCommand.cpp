#include "VulkanCommand.h"
#include "dkpch.h"

#include "Deako/Core/Application.h"

#include "VulkanFramebuffer.h"
#include "VulkanBuffer.h"

namespace Deako {

    const uint16_t INSTANCE_COUNT = 2;

    Ref<VulkanResources> CommandPool::s_VR = VulkanBase::GetResources();
    Ref<VulkanSettings> CommandPool::s_VS = VulkanBase::GetSettings();

    // Command pools manage memory that is used to store buffers and command buffers are allocated from them
    void CommandPool::Create()
    {
        // ImGui
        VkCommandPoolCreateInfo poolInfo = VulkanInitializers::CommandPoolCreateInfo();
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = s_VR->graphicsFamily.value();

        VK_CHECK_RESULT(vkCreateCommandPool(s_VR->device, &poolInfo, nullptr, &s_VR->imguiCommandPool));

        CommandPool::CreateBuffers(s_VR->imguiCommandPool, s_VR->imguiCommandBuffers);

        // Viewport
        VkCommandPoolCreateInfo viewportPoolInfo = VulkanInitializers::CommandPoolCreateInfo();
        viewportPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        viewportPoolInfo.queueFamilyIndex = s_VR->graphicsFamily.value();

        VK_CHECK_RESULT(vkCreateCommandPool(s_VR->device, &viewportPoolInfo, nullptr, &s_VR->viewportCommandPool));

        CommandPool::CreateBuffers(s_VR->viewportCommandPool, s_VR->viewportCommandBuffers);
    }

    void CommandPool::CleanUp()
    {
        vkDestroyCommandPool(s_VR->device, s_VR->imguiCommandPool, nullptr);
        vkDestroyCommandPool(s_VR->device, s_VR->viewportCommandPool, nullptr);
    }

    // Command buffers will be automatically freed when their command pool is destroyed
    void CommandPool::CreateBuffers(VkCommandPool commandPool, std::vector<VkCommandBuffer>& commandBuffers)
    {
        commandBuffers.resize(s_VS->imageCount);

        VkCommandBufferAllocateInfo allocInfo =
            VulkanInitializers::CommandBufferAllocateInfo(commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, (uint32_t)commandBuffers.size());
        // levels:
        // • VK_COMMAND_BUFFER_LEVEL_PRIMARY: can be submitted to a queue for execution, but cannot be called from other command buffers
        // • VK_COMMAND_BUFFER_LEVEL_SECONDARY: cannot be submitted directly, but can be called from primary command buffers

        VK_CHECK_RESULT(vkAllocateCommandBuffers(s_VR->device, &allocInfo, commandBuffers.data()));
    }

    void CommandPool::RecordViewportCommandBuffer(VkCommandBuffer commandBuffer, uint32_t currentFrame, uint32_t imageIndex)
    {
        vkResetCommandBuffer(commandBuffer, 0);

        // prepare recording to buffer
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr; // Optional

        // start recording to buffer
        VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        // prepare render pass
        VkRenderPassBeginInfo renderPassInfo = VulkanInitializers::RenderPassBeginInfo();
        renderPassInfo.renderPass = s_VR->viewportRenderPass;
        renderPassInfo.framebuffer = s_VR->viewportFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = s_VR->imageExtent;
        // define the clear values to use for VK_ATTACHMENT_LOAD_OP_CLEAR(load operation for the color attachment)
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        // start render pass
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_VR->viewportPipeline);

        VkViewport viewport =
            VulkanInitializers::Viewport(static_cast<float>(s_VR->imageExtent.width), static_cast<float>(s_VR->imageExtent.height), 0.0f, 1.0f);

        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = s_VR->imageExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        const Ref<VertexBuffer>& vertexBuffer = BufferPool::GetVertexBuffer();
        const Ref<IndexBuffer>& indexBuffer = BufferPool::GetIndexBuffer();
        const Ref<InstanceBuffer>& instanceBuffer = BufferPool::GetInstanceBuffer();

        VkBuffer vertexBuffers[] = { vertexBuffer->GetBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindVertexBuffers(commandBuffer, 1, 1, &instanceBuffer->GetBuffer(), offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_VR->pipelineLayout, 0, 1, &BufferPool::GetDescriptorSet(currentFrame), 0, nullptr);

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indexBuffer->GetIndices().size()), INSTANCE_COUNT, 0, 0, 0);

        // end the render pass
        vkCmdEndRenderPass(commandBuffer);

        VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
    }

    void CommandPool::RecordImGuiCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        vkResetCommandBuffer(commandBuffer, 0);

        // prepare recording to buffer
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // required for imgui
        beginInfo.pInheritanceInfo = nullptr; // Optional

        // start recording to buffer
        VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        // prepare render pass
        VkRenderPassBeginInfo renderPassInfo = VulkanInitializers::RenderPassBeginInfo();
        renderPassInfo.renderPass = s_VR->imguiRenderPass;
        // need to bind the framebuffer for the swapchain image we want to draw to, using the passed in imageIndex
        renderPassInfo.framebuffer = s_VR->imguiFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = s_VR->imageExtent;
        // define the clear values to use for VK_ATTACHMENT_LOAD_OP_CLEAR(load operation for the color attachment)
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        // start render pass
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        LayerStack& layerStack = Application::Get().GetLayerStack();
        ImGuiLayer* imguiLayer = Application::Get().GetImGuiLayer();

        imguiLayer->Begin();
        for (Layer* layer : layerStack)
            layer->OnImGuiRender();
        imguiLayer->End(commandBuffer);

        // end the render pass
        vkCmdEndRenderPass(commandBuffer);

        VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
    }

    VkCommandBuffer CommandPool::BeginSingleTimeCommands(VkCommandPool commandPool)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(s_VR->device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void CommandPool::EndSingleTimeCommands(VkCommandPool commandPool, VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(s_VR->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

        vkQueueWaitIdle(s_VR->graphicsQueue);

        vkFreeCommandBuffers(s_VR->device, commandPool, 1, &commandBuffer);
    }

}
