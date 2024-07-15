#include "VulkanCommand.h"
#include "dkpch.h"

#include "Deako/Core/Application.h"

#include "VulkanFramebuffer.h"
#include "VulkanBuffer.h"

namespace Deako {

    Ref<VulkanResources> CommandPool::s_VR = VulkanBase::GetResources();
    Ref<VulkanSettings> CommandPool::s_VS = VulkanBase::GetSettings();

    // Command pools manage memory that is used to store buffers and command buffers are allocated from them
    void CommandPool::Create()
    {
        VkCommandPoolCreateInfo poolInfo =
            VulkanInitializers::CommandPoolCreateInfo();
        // two possible flags for command pools:
        // • VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior)
        // • VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: allow command buffers to be rerecorded individually, without this flag they all have to be reset together
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        // Command buffers are executed by submitting them on one of the device queues, like the graphics and presentation queues. Each command pool can only allocate command buffers that are submitted on a single type of queue
        poolInfo.queueFamilyIndex = s_VR->graphicsFamily.value();

        VkResult result = vkCreateCommandPool(s_VR->device, &poolInfo, nullptr, &s_VR->commandPool);
        DK_CORE_ASSERT(!result);

        CommandPool::CreateBuffers(s_VR->commandPool, s_VR->commandBuffers);

        // Viewport
        VkCommandPoolCreateInfo viewportPoolInfo =
            VulkanInitializers::CommandPoolCreateInfo();
        viewportPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        viewportPoolInfo.queueFamilyIndex = s_VR->graphicsFamily.value();

        result = vkCreateCommandPool(s_VR->device, &viewportPoolInfo, nullptr, &s_VR->viewportCommandPool);
        DK_CORE_ASSERT(!result);

        CommandPool::CreateBuffers(s_VR->viewportCommandPool, s_VR->viewportCommandBuffers);
    }

    void CommandPool::CleanUp()
    {
        vkDestroyCommandPool(s_VR->device, s_VR->commandPool, nullptr);
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

        VkResult result = vkAllocateCommandBuffers(s_VR->device, &allocInfo, commandBuffers.data());
        DK_CORE_ASSERT(!result);
    }

    VkCommandBuffer CommandPool::Record(uint32_t currentFrame, uint32_t imageIndex)
    {
        VkCommandBuffer commandBuffer = s_VR->commandBuffers[currentFrame];

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
        renderPassInfo.renderPass = s_VR->renderPass;
        // need to bind the framebuffer for the swapchain image we want to draw to, using the passed in imageIndex
        renderPassInfo.framebuffer = s_VR->framebuffers[imageIndex];
        // define the size of the render area
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = s_VR->imageExtent;
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
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_VR->graphicsPipeline);


        // ImGui
        LayerStack& layerStack = Application::Get().GetLayerStack();
        ImGuiLayer* imguiLayer = Application::Get().GetImGuiLayer();

        imguiLayer->Begin();
        for (Layer* layer : layerStack)
            layer->OnImGuiRender();
        imguiLayer->End(commandBuffer, s_VR->graphicsPipeline);

        // end the render pass
        vkCmdEndRenderPass(commandBuffer);

        result = vkEndCommandBuffer(commandBuffer);
        DK_CORE_ASSERT(!result);

        return commandBuffer;
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
