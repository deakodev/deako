#include "VulkanPicker.h"
#include "dkpch.h"

#include "VulkanBase.h"
#include "VulkanScene.h"
#include "VulkanResource.h"
#include "VulkanPipeline.h"

#include "Deako/Asset/Scene/Entity.h"
#include "Deako/ImGui/ImGuiLayer.h" 

namespace Deako {

    void VulkanPicker::Init()
    {
        SetUpTargets();

        SetUpUniforms();

        SetUpDescriptors();

        SetUpPipeline();
    }

    void VulkanPicker::CleanUp()
    {
        VulkanPickerResources& vp = s_Resources;
        VulkanBaseResources& vb = VulkanBase::GetResources();

        vkDestroyPipeline(vb.device, vp.pipeline, nullptr);
        vkDestroyPipelineLayout(vb.device, vp.pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(vb.device, vp.descriptorLayout, nullptr);

        vp.staticDescriptorAllocator.DestroyPools();

        for (auto& uniform : vp.uniforms)
            VulkanBuffer::Destroy(uniform.buffer);

        VulkanImage::Destroy(vp.colorTarget);
        VulkanImage::Destroy(vp.depthTarget);
        VulkanBuffer::Destroy(vp.stagingBuffer);
    }

    void VulkanPicker::OnUpdate()
    {
        VulkanPickerResources& vp = s_Resources;
        VulkanBaseResources& vb = VulkanBase::GetResources();

        DkU32 index = 0;
        for (auto& entity : GetActiveScene().GetEntities())
        {
            DkVec4* colorID = (DkVec4*)(((DkU64)vp.uniformDynamicData.colorID) + (index * vp.dynamicUniformAlignment));
            *colorID = entity.GetPickerColor();
            index++;
        }

        UniformBuffer uniform = vp.uniforms[vb.context.currentFrame];
        memcpy(uniform.buffer.mapped, vp.uniformDynamicData.colorID, vp.dynamicUniformAlignment * GetActiveScene().GetEntities().size());

        // flush dynamic uniform to make changes visible to the host
        VkMappedMemoryRange memoryRange{};
        memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        memoryRange.memory = uniform.buffer.memory;
        memoryRange.size = vp.dynamicUniformAlignment * GetActiveScene().GetEntities().size();
        vkFlushMappedMemoryRanges(vb.device, 1, &memoryRange);
    }

    void VulkanPicker::SetUpTargets()
    {
        VulkanPickerResources& vp = s_Resources;
        VulkanBaseResources& vb = VulkanBase::GetResources();

        VkExtent3D targetExtent = { vb.swapchain.extent.width, vb.swapchain.extent.height, 1 };
        VkSampleCountFlagBits targetSamples = VK_SAMPLE_COUNT_1_BIT;

        VkFormat colorFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
        VkImageUsageFlags colorUsages = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        vp.colorTarget =
            VulkanImage::Create(targetExtent, colorFormat, targetSamples, colorUsages, 1, VK_IMAGE_TYPE_2D);

        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
        VkImageUsageFlags depthUsages = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        vp.depthTarget =
            VulkanImage::Create(targetExtent, depthFormat, targetSamples, depthUsages, 1, VK_IMAGE_TYPE_2D);

        vp.stagingBuffer = VulkanBuffer::Create(sizeof(DkVec4), VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkCR(vkMapMemory(vb.device, vp.stagingBuffer.memory, 0, sizeof(DkVec4), 0, &vp.stagingBuffer.mapped));
    }

    void VulkanPicker::SetUpUniforms()
    {
        VulkanPickerResources& vp = s_Resources;
        VulkanBaseResources& vb = VulkanBase::GetResources();

        // dynamic uniform alignments
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(vb.physicalDevice, &deviceProperties);

        // determine required alignment based on min device offset alignment
        size_t minUniformAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
        vp.dynamicUniformAlignment = sizeof(DkVec4);

        if (minUniformAlignment > 0)
            vp.dynamicUniformAlignment = (vp.dynamicUniformAlignment + minUniformAlignment - 1) & ~(minUniformAlignment - 1);

        size_t pickerBufferSize = GetActiveScene().GetEntities().size() * vp.dynamicUniformAlignment;

        vp.uniformDynamicData.colorID = (DkVec4*)VulkanMemory::AlignedAlloc(pickerBufferSize, vp.dynamicUniformAlignment);

        DK_CORE_ASSERT(vp.uniformDynamicData.colorID, "Failed to allocate colorID uniform dynamic data!");

        vp.uniforms.resize(vb.swapchain.imageCount);
        for (auto& uniform : vp.uniforms)
        {   // dynamic uniform buffer object with colorID
            uniform.buffer = VulkanBuffer::Create(pickerBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uniform.descriptor = { uniform.buffer.buffer, 0, vp.dynamicUniformAlignment };
            VkCR(vkMapMemory(vb.device, uniform.buffer.memory, 0, pickerBufferSize, 0, &uniform.buffer.mapped));
        }
    }

    void VulkanPicker::SetUpDescriptors()
    {
        VulkanPickerResources& vp = s_Resources;
        VulkanSceneResources& vs = VulkanScene::GetActiveSceneResources();
        ProjectAssetPool& assetPool = Deako::GetProjectAssetPool();

        {   // static descriptor pool allocator
            DkU32 meshCount = 0;
            DkU32 materialCount = 0;
            DkU32 materialSamplerCount = 0;

            for (auto& entity : GetActiveScene().GetEntities())
            {
                if (entity.HasComponent<PrefabComponent>())
                {
                    auto& prefabComp = entity.GetComponent<PrefabComponent>();
                    Ref<Model> model = assetPool.GetAsset<Model>(prefabComp.meshHandle);

                    for (auto node : model->linearNodes)
                        if (node->mesh) meshCount++; // 1 set for mesh

                    DkU32 entityMaterialCount = prefabComp.materialHandles.size();
                    materialCount += entityMaterialCount;
                    materialSamplerCount += (entityMaterialCount * 5); // 5 sets for material samplers
                }
            }

            std::vector<VulkanDescriptor::PoolSizeRatio> poolSizes = {
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 2 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,(DkF32)meshCount },
            };

            DkU32 maxSets = meshCount + materialCount + materialSamplerCount + 1;
            VulkanDescriptor::AllocatorGrowable descriptorAllocator{ maxSets, poolSizes };
            vp.staticDescriptorAllocator = descriptorAllocator;
        }

        {
            VulkanDescriptor::LayoutBuilder layoutBuilder;
            layoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT);
            layoutBuilder.AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
            layoutBuilder.AddBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);

            vp.descriptorLayout = layoutBuilder.Build();
            VkDescriptorSetLayout layout = vp.descriptorLayout;

            vp.descriptorSet = vp.staticDescriptorAllocator.Allocate(layout);
            VkDescriptorSet set = vp.descriptorSet;

            UniformBuffer& pickerUniform = vp.uniforms[0]; // any of the dynamic descs should work?
            UniformSet& sceneUniform = vs.uniforms[0];

            VulkanDescriptor::Writer writer;
            writer.WriteBuffer(0, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, pickerUniform.descriptor);
            writer.WriteBuffer(1, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, sceneUniform.dynamic.descriptor);
            writer.WriteBuffer(2, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, sceneUniform.shared.descriptor);

            writer.UpdateSets();
        }
    }

    void VulkanPicker::SetUpPipeline()
    {
        VulkanPickerResources& vp = s_Resources;
        VulkanBaseResources& vb = VulkanBase::GetResources();
        VulkanSceneResources& vs = VulkanScene::GetActiveSceneResources();

        VkShaderModule pickerVert = VulkanShader::CreateModule("picker.vert.spv");
        VkShaderModule pickerFrag = VulkanShader::CreateModule("picker.frag.spv");

        std::vector<VkDescriptorSetLayout> pickerDescriptorLayouts = {
             vp.descriptorLayout,
             vs.descriptorLayouts.node,
        };

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.size = sizeof(DkU32);
        pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        pipelineLayoutInfo.setLayoutCount = static_cast<DkU32>(pickerDescriptorLayouts.size());
        pipelineLayoutInfo.pSetLayouts = pickerDescriptorLayouts.data();
        VkCR(vkCreatePipelineLayout(vb.device, &pipelineLayoutInfo, nullptr, &vp.pipelineLayout));

        VkVertexInputBindingDescription vertexInputBinding = { 0, sizeof(Model::Vertex), VK_VERTEX_INPUT_RATE_VERTEX };
        std::vector<VkVertexInputAttributeDescription>  vertexInputAttributes;

        {
            VulkanPipeline::Builder pipelineBuilder;
            pipelineBuilder.SetShaders(pickerVert, pickerFrag);
            pipelineBuilder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
            pipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
            pipelineBuilder.SetCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
            pipelineBuilder.DisableColorBlending();
            pipelineBuilder.SetMultisampling(VK_SAMPLE_COUNT_1_BIT);
            pipelineBuilder.EnableDepthTest();

            vertexInputAttributes = {
               { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Model::Vertex, pos) },
               { 1, 0, VK_FORMAT_R32G32B32A32_UINT, offsetof(Model::Vertex, joint0) },
               { 2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Model::Vertex, weight0) },
            };

            pipelineBuilder.SetVertexInput(vertexInputBinding, vertexInputAttributes);

            vp.pipeline = pipelineBuilder.Build(vp.pipelineLayout, &vp.colorTarget.format);
        }

        vkDestroyShaderModule(vb.device, pickerVert, nullptr);
        vkDestroyShaderModule(vb.device, pickerFrag, nullptr);
    }

    void VulkanPicker::Draw(VkCommandBuffer commandBuffer, DkU32 imageIndex)
    {   // offscreen color picking draw
        VulkanPickerResources& vp = s_Resources;
        VulkanBaseResources& vb = VulkanBase::GetResources();
        VulkanSceneResources& vs = VulkanScene::GetActiveSceneResources();

        DkInput& di = Deako::GetInput();
        if (AreEventsBlocked() || !IsMousePositionValid(di.mousePosition)) return;

        VulkanImage::Transition(commandBuffer, vp.colorTarget.image, vp.colorTarget.format, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        VulkanImage::Transition(commandBuffer, vp.depthTarget.image, vp.depthTarget.format, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

        VkRenderingAttachmentInfo colorAttachment = {};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.pNext = nullptr;
        colorAttachment.imageView = vp.colorTarget.view;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };

        VkRenderingAttachmentInfo depthStencilAttachment = {};
        depthStencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthStencilAttachment.pNext = nullptr;
        depthStencilAttachment.imageView = vp.depthTarget.view;
        depthStencilAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthStencilAttachment.clearValue.depthStencil = { 1.0f,  0 };

        VkRenderingInfo renderInfo = {};
        renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderInfo.pNext = nullptr;
        renderInfo.renderArea = { VkOffset2D{ 0, 0}, { vb.swapchain.extent.width, vb.swapchain.extent.height } };
        renderInfo.layerCount = 1;
        renderInfo.colorAttachmentCount = 1;
        renderInfo.pColorAttachments = &colorAttachment;
        renderInfo.pDepthAttachment = &depthStencilAttachment;
        renderInfo.pStencilAttachment = &depthStencilAttachment;

        vkCmdBeginRenderingKHR(commandBuffer, &renderInfo);

        VkViewport viewport{};
        viewport.width = vb.swapchain.extent.width;
        viewport.height = vb.swapchain.extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = { vb.swapchain.extent.width, vb.swapchain.extent.height };
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vp.pipeline);

        DkU32 index = 0;
        auto DrawNode = [&](Node* node)
            {
                if (!node->mesh) return;
                for (Primitive* primitive : node->mesh->primitives)
                {
                    if (primitive->material.alphaMode == Material::ALPHAMODE_OPAQUE)
                    {
                        DkU32 dynamicOffsets[2] = {
                            index * static_cast<DkU32>(vp.dynamicUniformAlignment),
                            index * static_cast<DkU32>(vs.dynamicUniformAlignment)
                        };

                        const std::vector<VkDescriptorSet> descriptorSets = {
                            vp.descriptorSet,
                            node->mesh->uniform.descriptorSet,
                        };

                        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vp.pipelineLayout, 0, static_cast<DkU32>(descriptorSets.size()), descriptorSets.data(), 2, dynamicOffsets);

                        if (primitive->hasIndices)
                            vkCmdDrawIndexed(commandBuffer, primitive->indexCount, 1, primitive->firstIndex, 0, 0);
                        else
                            vkCmdDraw(commandBuffer, primitive->vertexCount, 1, 0, 0);
                    }
                }
            };

        ProjectAssetPool& projectAssetPool = Deako::GetProjectAssetPool();
        for (auto& entity : GetActiveScene().GetEntities())
        {
            if (entity.HasComponent<PrefabComponent>())
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

        VulkanImage::Transition(commandBuffer, vp.colorTarget.image, vp.colorTarget.format, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        vkCmdCopyImageToBuffer(commandBuffer, vp.colorTarget.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vp.stagingBuffer.buffer, 1, &copyRegion);
    }

}
