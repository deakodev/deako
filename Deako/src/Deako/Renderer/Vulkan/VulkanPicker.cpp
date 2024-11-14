#include "VulkanPicker.h"
#include "dkpch.h"

#include "VulkanBase.h"
#include "VulkanScene.h"
#include "VulkanResource.h"

#include "Deako/ImGui/ImGuiLayer.h" 

namespace Deako {

    static Ref<VulkanBaseResources> vb = VulkanBase::GetResources();
    static Ref<VulkanSceneResources> vs = VulkanScene::GetResources();

    void VulkanPicker::Init()
    {
        SetUpTargets();

        // TODO: extract picker impl from VulkanScene
        // SetUpUniforms();

        // SetUpDescriptors();

        // SetUpPipelines();
    }

    void VulkanPicker::CleanUp()
    {
        vkDestroyPipeline(vb->device, vs->pipelines.picker, nullptr);
        vkDestroyPipelineLayout(vb->device, vs->pickerPipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(vb->device, vs->descriptorLayouts.picker, nullptr);

        VulkanImage::Destroy(vs->picker.colorTarget);
        VulkanImage::Destroy(vs->picker.depthTarget);
        VulkanBuffer::Destroy(vs->picker.stagingBuffer);
    }

    void VulkanPicker::SetUpTargets()
    {
        VkExtent3D targetExtent = { vb->swapchain.extent.width, vb->swapchain.extent.height, 1 };
        VkSampleCountFlagBits targetSamples = VK_SAMPLE_COUNT_1_BIT;

        VkFormat colorFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
        VkImageUsageFlags colorUsages = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        vs->picker.colorTarget =
            VulkanImage::Create(targetExtent, colorFormat, targetSamples, colorUsages, 1, VK_IMAGE_TYPE_2D);

        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
        VkImageUsageFlags depthUsages = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        vs->picker.depthTarget =
            VulkanImage::Create(targetExtent, depthFormat, targetSamples, depthUsages, 1, VK_IMAGE_TYPE_2D);

        vs->picker.stagingBuffer = VulkanBuffer::Create(sizeof(DkVec4), VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkCR(vkMapMemory(vb->device, vs->picker.stagingBuffer.memory, 0, sizeof(DkVec4), 0, &vs->picker.stagingBuffer.mapped));
    }


    void VulkanPicker::Draw(VkCommandBuffer commandBuffer, DkU32 imageIndex)
    {   // offscreen color picking draw
        DkInput& di = Deako::GetInput();
        if (AreEventsBlocked() || !IsMousePositionValid(di.mousePosition)) return;

        VulkanImage::Transition(commandBuffer, vs->picker.colorTarget.image, vs->picker.colorTarget.format, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        VulkanImage::Transition(commandBuffer, vs->picker.depthTarget.image, vs->picker.depthTarget.format, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

        VkRenderingAttachmentInfo colorAttachment = {};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.pNext = nullptr;
        colorAttachment.imageView = vs->picker.colorTarget.view;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };

        VkRenderingAttachmentInfo depthStencilAttachment = {};
        depthStencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthStencilAttachment.pNext = nullptr;
        depthStencilAttachment.imageView = vs->picker.depthTarget.view;
        depthStencilAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthStencilAttachment.clearValue.depthStencil = { 1.0f,  0 };

        VkRenderingInfo renderInfo = {};
        renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderInfo.pNext = nullptr;
        renderInfo.renderArea = { VkOffset2D{ 0, 0}, { vb->swapchain.extent.width, vb->swapchain.extent.height } };
        renderInfo.layerCount = 1;
        renderInfo.colorAttachmentCount = 1;
        renderInfo.pColorAttachments = &colorAttachment;
        renderInfo.pDepthAttachment = &depthStencilAttachment;
        renderInfo.pStencilAttachment = &depthStencilAttachment;

        vkCmdBeginRenderingKHR(commandBuffer, &renderInfo);

        VkViewport viewport{};
        viewport.width = vb->swapchain.extent.width;
        viewport.height = vb->swapchain.extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = { vb->swapchain.extent.width, vb->swapchain.extent.height };
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vs->pipelines.picker);

        DkU32 index = 0;
        auto DrawNode = [&](Node* node)
            {
                if (!node->mesh) return;
                for (Primitive* primitive : node->mesh->primitives)
                {
                    if (primitive->material.alphaMode == Material::ALPHAMODE_OPAQUE)
                    {
                        DkU32 dynamicOffsets[2] = {
                            index * static_cast<DkU32>(vs->pickerUniformAlignment),
                            index * static_cast<DkU32>(vs->dynamicUniformAlignment)
                        };

                        const std::vector<VkDescriptorSet> descriptorSets = {
                            vs->picker.descriptorSet,
                            node->mesh->uniform.descriptorSet,
                        };

                        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vs->pickerPipelineLayout, 0, static_cast<DkU32>(descriptorSets.size()), descriptorSets.data(), 2, dynamicOffsets);

                        if (primitive->hasIndices)
                            vkCmdDrawIndexed(commandBuffer, primitive->indexCount, 1, primitive->firstIndex, 0, 0);
                        else
                            vkCmdDraw(commandBuffer, primitive->vertexCount, 1, 0, 0);
                    }
                }
            };

        Scene& activeScene = Deako::GetActiveScene();
        ProjectAssetPool& projectAssetPool = Deako::GetProjectAssetPool();
        for (auto& entity : activeScene.entities)
        {
            auto& prefabComp = entity.GetComponent<PrefabComponent>();
            Ref<Model> model = projectAssetPool.GetAsset<Model>(prefabComp.meshHandle);

            VkDeviceSize vertexOffset = 0;
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model->vertices.buffer, &vertexOffset);

            if (model->indices.buffer != VK_NULL_HANDLE)
                vkCmdBindIndexBuffer(commandBuffer, model->indices.buffer, 0, VK_INDEX_TYPE_UINT32);

            // render each node with picking color
            for (auto node : model->nodes)
            {
                DrawNode(node);

                for (auto child : node->children)
                    DrawNode(child);
            }

            index++;
        }

        vkCmdEndRenderingKHR(commandBuffer);

        VkBufferImageCopy copyRegion{};
        copyRegion.bufferOffset = 0;
        copyRegion.bufferRowLength = 0;
        copyRegion.bufferImageHeight = 0;
        copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.imageSubresource.mipLevel = 0;
        copyRegion.imageSubresource.baseArrayLayer = 0;
        copyRegion.imageSubresource.layerCount = 1;
        copyRegion.imageOffset = { static_cast<int32_t>(di.mousePosition.x), static_cast<int32_t>(di.mousePosition.y), 0 }; // copy from exact pixel position
        copyRegion.imageExtent = { 1, 1, 1 };

        VulkanImage::Transition(commandBuffer, vs->picker.colorTarget.image, vs->picker.colorTarget.format, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        vkCmdCopyImageToBuffer(commandBuffer, vs->picker.colorTarget.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vs->picker.stagingBuffer.buffer, 1, &copyRegion);
    }

    const DkVec4& VulkanPicker::GetPixelColor()
    {
        return *static_cast<DkVec4*>(vs->picker.stagingBuffer.mapped);
    }

}
