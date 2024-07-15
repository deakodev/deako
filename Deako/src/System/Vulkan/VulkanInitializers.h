#pragma once

#include "vulkan/vulkan.h"

namespace Deako
{
    namespace VulkanInitializers
    {

        inline VkMemoryAllocateInfo MemoryAllocateInfo()
        {
            VkMemoryAllocateInfo memAllocInfo{};
            memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            return memAllocInfo;
        }

        inline VkImageCreateInfo ImageCreateInfo()
        {
            VkImageCreateInfo imageCreateInfo{};
            imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            return imageCreateInfo;
        }

        inline VkImageViewCreateInfo ImageViewCreateInfo()
        {
            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            return imageViewCreateInfo;
        }

        inline VkSamplerCreateInfo SamplerCreateInfo()
        {
            VkSamplerCreateInfo samplerCreateInfo{};
            samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            return samplerCreateInfo;
        }

        inline VkRenderPassCreateInfo RenderPassCreateInfo()
        {
            VkRenderPassCreateInfo renderPassCreateInfo{};
            renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            return renderPassCreateInfo;
        }

        inline VkRenderPassBeginInfo RenderPassBeginInfo()
        {
            VkRenderPassBeginInfo renderPassBeginInfo{};
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            return renderPassBeginInfo;
        }

        /** @brief Initialize an image memory barrier with no image transfer ownership */
        inline VkImageMemoryBarrier ImageMemoryBarrier()
        {
            VkImageMemoryBarrier imageMemoryBarrier{};
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            return imageMemoryBarrier;
        }

        inline VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo(
            const std::vector<VkDescriptorPoolSize>& poolSizes,
            uint32_t maxSets)
        {
            VkDescriptorPoolCreateInfo descriptorPoolInfo{};
            descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            descriptorPoolInfo.pPoolSizes = poolSizes.data();
            descriptorPoolInfo.maxSets = maxSets;
            return descriptorPoolInfo;
        }

        inline VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(
            VkDescriptorType type,
            VkShaderStageFlags stageFlags,
            uint32_t binding,
            uint32_t descriptorCount = 1)
        {
            VkDescriptorSetLayoutBinding setLayoutBinding{};
            setLayoutBinding.descriptorType = type;
            setLayoutBinding.stageFlags = stageFlags;
            setLayoutBinding.binding = binding;
            setLayoutBinding.descriptorCount = descriptorCount;
            return setLayoutBinding;
        }

        inline VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo(
            const std::vector<VkDescriptorSetLayoutBinding>& bindings)
        {
            VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
            descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descriptorSetLayoutCreateInfo.pBindings = bindings.data();
            descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
            return descriptorSetLayoutCreateInfo;
        }

        inline VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo(
            VkDescriptorPool descriptorPool,
            const VkDescriptorSetLayout* pSetLayouts,
            uint32_t descriptorSetCount)
        {
            VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
            descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            descriptorSetAllocateInfo.descriptorPool = descriptorPool;
            descriptorSetAllocateInfo.pSetLayouts = pSetLayouts;
            descriptorSetAllocateInfo.descriptorSetCount = descriptorSetCount;
            return descriptorSetAllocateInfo;
        }

        inline VkVertexInputBindingDescription VertexInputBindingDescription(
            uint32_t binding,
            uint32_t stride,
            VkVertexInputRate inputRate)
        {
            VkVertexInputBindingDescription vInputBindDescription{};
            vInputBindDescription.binding = binding;
            vInputBindDescription.stride = stride;
            vInputBindDescription.inputRate = inputRate;
            return vInputBindDescription;
        }

        inline VkVertexInputAttributeDescription VertexInputAttributeDescription(
            uint32_t binding,
            uint32_t location,
            VkFormat format,
            uint32_t offset)
        {
            VkVertexInputAttributeDescription vInputAttribDescription{};
            vInputAttribDescription.location = location;
            vInputAttribDescription.binding = binding;
            vInputAttribDescription.format = format;
            vInputAttribDescription.offset = offset;
            return vInputAttribDescription;
        }

        inline VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateCreateInfo(
            const std::vector<VkVertexInputBindingDescription>& vertexBindingDescriptions,
            const std::vector<VkVertexInputAttributeDescription>& vertexAttributeDescriptions
        )
        {
            VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
            pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDescriptions.size());
            pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vertexBindingDescriptions.data();
            pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.size());
            pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();
            return pipelineVertexInputStateCreateInfo;
        }

        inline VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo(
            VkPrimitiveTopology topology,
            VkPipelineInputAssemblyStateCreateFlags flags,
            VkBool32 primitiveRestartEnable)
        {
            VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
            pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            pipelineInputAssemblyStateCreateInfo.topology = topology;
            pipelineInputAssemblyStateCreateInfo.flags = flags;
            pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = primitiveRestartEnable;
            return pipelineInputAssemblyStateCreateInfo;
        }

        inline VkPipelineDynamicStateCreateInfo PipelineDynamicStateCreateInfo(
            const std::vector<VkDynamicState>& pDynamicStates,
            VkPipelineDynamicStateCreateFlags flags = 0)
        {
            VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
            pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            pipelineDynamicStateCreateInfo.pDynamicStates = pDynamicStates.data();
            pipelineDynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(pDynamicStates.size());
            pipelineDynamicStateCreateInfo.flags = flags;
            return pipelineDynamicStateCreateInfo;
        }

        inline VkViewport Viewport(
            float width,
            float height,
            float minDepth,
            float maxDepth)
        {
            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = width;
            viewport.height = height;
            viewport.minDepth = minDepth;
            viewport.maxDepth = maxDepth;
            return viewport;
        }

        inline VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo(
            uint32_t viewportCount = 1,
            uint32_t scissorCount = 1,
            VkPipelineViewportStateCreateFlags flags = 0)
        {
            VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
            pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            pipelineViewportStateCreateInfo.viewportCount = viewportCount;
            pipelineViewportStateCreateInfo.scissorCount = scissorCount;
            pipelineViewportStateCreateInfo.flags = flags;
            return pipelineViewportStateCreateInfo;
        }

        inline VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo(
            VkPolygonMode polygonMode,
            VkCullModeFlags cullMode,
            VkFrontFace frontFace,
            VkPipelineRasterizationStateCreateFlags flags = 0)
        {
            VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
            pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            pipelineRasterizationStateCreateInfo.polygonMode = polygonMode;
            pipelineRasterizationStateCreateInfo.cullMode = cullMode;
            pipelineRasterizationStateCreateInfo.frontFace = frontFace;
            pipelineRasterizationStateCreateInfo.flags = flags;
            pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
            pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
            return pipelineRasterizationStateCreateInfo;
        }

        inline VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo(
            VkSampleCountFlagBits rasterizationSamples,
            VkPipelineMultisampleStateCreateFlags flags = 0)
        {
            VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
            pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            pipelineMultisampleStateCreateInfo.rasterizationSamples = rasterizationSamples;
            pipelineMultisampleStateCreateInfo.flags = flags;
            return pipelineMultisampleStateCreateInfo;
        }

        inline VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo(
            VkBool32 depthTestEnable,
            VkBool32 depthWriteEnable,
            VkCompareOp depthCompareOp)
        {
            VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
            pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            pipelineDepthStencilStateCreateInfo.depthTestEnable = depthTestEnable;
            pipelineDepthStencilStateCreateInfo.depthWriteEnable = depthWriteEnable;
            pipelineDepthStencilStateCreateInfo.depthCompareOp = depthCompareOp;
            pipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
            return pipelineDepthStencilStateCreateInfo;
        }

        inline VkPipelineColorBlendAttachmentState PipelineColorBlendAttachmentState(
            VkColorComponentFlags colorWriteMask,
            VkBool32 blendEnable)
        {
            VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};
            pipelineColorBlendAttachmentState.colorWriteMask = colorWriteMask;
            pipelineColorBlendAttachmentState.blendEnable = blendEnable;
            return pipelineColorBlendAttachmentState;
        }

        inline VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfo(
            uint32_t attachmentCount,
            const VkPipelineColorBlendAttachmentState* pAttachments)
        {
            VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
            pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            pipelineColorBlendStateCreateInfo.attachmentCount = attachmentCount;
            pipelineColorBlendStateCreateInfo.pAttachments = pAttachments;
            return pipelineColorBlendStateCreateInfo;
        }

        inline VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo(
            const VkDescriptorSetLayout* pSetLayouts,
            uint32_t setLayoutCount = 1)
        {
            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
            pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutCreateInfo.setLayoutCount = setLayoutCount;
            pipelineLayoutCreateInfo.pSetLayouts = pSetLayouts;
            return pipelineLayoutCreateInfo;
        }

        inline VkGraphicsPipelineCreateInfo PipelineCreateInfo(
            VkPipelineLayout layout,
            VkRenderPass renderPass,
            VkPipelineCreateFlags flags = 0)
        {
            VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
            pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineCreateInfo.layout = layout;
            pipelineCreateInfo.renderPass = renderPass;
            pipelineCreateInfo.flags = flags;
            pipelineCreateInfo.basePipelineIndex = -1;
            pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
            return pipelineCreateInfo;
        }

        inline VkCommandPoolCreateInfo CommandPoolCreateInfo()
        {
            VkCommandPoolCreateInfo cmdPoolCreateInfo{};
            cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            return cmdPoolCreateInfo;
        }

        inline VkCommandBufferAllocateInfo CommandBufferAllocateInfo(
            VkCommandPool commandPool,
            VkCommandBufferLevel level,
            uint32_t bufferCount)
        {
            VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
            commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            commandBufferAllocateInfo.commandPool = commandPool;
            commandBufferAllocateInfo.level = level;
            commandBufferAllocateInfo.commandBufferCount = bufferCount;
            return commandBufferAllocateInfo;
        }

        inline VkFramebufferCreateInfo FramebufferCreateInfo()
        {
            VkFramebufferCreateInfo framebufferCreateInfo{};
            framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            return framebufferCreateInfo;
        }

    }
}
