#include "VulkanPipeline.h"
#include "dkpch.h"

#include "VulkanBase.h"

namespace Deako {

    static Ref<VulkanBaseResources> vb = VulkanBase::GetResources();

    namespace VulkanPipeline {

        VkPipeline Builder::Build(VkPipelineLayout pipelineLayout, const VkFormat* colorAttachmentFormats)
        {
            VkPipelineViewportStateCreateInfo viewportState{};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = 1;
            viewportState.scissorCount = 1;
            viewportState.pNext = nullptr;

            std::vector<VkDynamicState> dynamicEnables = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
            };

            VkPipelineDynamicStateCreateInfo dynamicState{};
            dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.dynamicStateCount = static_cast<DkU32>(dynamicEnables.size());
            dynamicState.pDynamicStates = dynamicEnables.data();

            VkPipelineRenderingCreateInfoKHR renderingInfo{};
            renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
            renderingInfo.colorAttachmentCount = 1;
            renderingInfo.pColorAttachmentFormats = colorAttachmentFormats;
            renderingInfo.depthAttachmentFormat = vb->swapchain.depthTarget.format; // TODO: add depth format variable
            renderingInfo.stencilAttachmentFormat = vb->swapchain.depthTarget.format;

            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.layout = pipelineLayout;

            pipelineInfo.stageCount = static_cast<DkU32>(m_ShaderStages.size());
            pipelineInfo.pStages = m_ShaderStages.data();
            pipelineInfo.pInputAssemblyState = &m_InputAssembly;
            pipelineInfo.pVertexInputState = &m_VertexInput;
            pipelineInfo.pRasterizationState = &m_Rasterizer;
            pipelineInfo.pColorBlendState = &m_ColorBlend;
            pipelineInfo.pMultisampleState = &m_Multisampling;
            pipelineInfo.pDepthStencilState = &m_DepthStencil;

            pipelineInfo.pViewportState = &viewportState;
            pipelineInfo.pDynamicState = &dynamicState;
            pipelineInfo.pNext = &renderingInfo;

            VkPipeline pipeline{};
            VkCR(vkCreateGraphicsPipelines(vb->device, vb->pipelineCache, 1, &pipelineInfo, nullptr, &pipeline));

            return pipeline;
        }

        void Builder::SetShaders(VkShaderModule vertexModule, VkShaderModule fragmentModule)
        {
            m_ShaderStages.clear();

            VkPipelineShaderStageCreateInfo shaderStage{};
            shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStage.module = vertexModule;
            shaderStage.pName = "main";
            shaderStage.pNext = nullptr;

            m_ShaderStages.push_back(shaderStage);

            shaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStage.module = fragmentModule;
            shaderStage.pName = "main";
            shaderStage.pNext = nullptr;

            m_ShaderStages.push_back(shaderStage);
        }

        void Builder::SetInputTopology(VkPrimitiveTopology topology)
        {
            m_InputAssembly.topology = topology;
            m_InputAssembly.primitiveRestartEnable = VK_FALSE;
        }

        void Builder::SetPolygonMode(VkPolygonMode mode)
        {
            m_Rasterizer.polygonMode = mode;
            m_Rasterizer.lineWidth = 1.0f;
        }

        void Builder::SetCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace)
        {
            m_Rasterizer.cullMode = cullMode;
            m_Rasterizer.frontFace = frontFace;
        }

        void Builder::SetMultisampling(VkSampleCountFlagBits sampleFlag)
        {
            m_Multisampling.rasterizationSamples = sampleFlag;
        }

        void Builder::DisableColorBlending()
        {
            m_ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            m_ColorBlendAttachment.blendEnable = VK_FALSE;

            m_ColorBlend.attachmentCount = 1;
            m_ColorBlend.pAttachments = &m_ColorBlendAttachment;
            m_ColorBlend.pNext = nullptr;
        }

        void Builder::EnableAlphaBlending()
        {
            m_ColorBlendAttachment.blendEnable = VK_TRUE;
            m_ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            m_ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            m_ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            m_ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            m_ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            m_ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            m_ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

            m_ColorBlend.attachmentCount = 1;
            m_ColorBlend.pAttachments = &m_ColorBlendAttachment;
            m_ColorBlend.pNext = nullptr;
        }

        void Builder::DisableDepthTest()
        {
            m_DepthStencil.depthTestEnable = VK_FALSE;
            m_DepthStencil.depthWriteEnable = VK_FALSE;
            m_DepthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
            m_DepthStencil.front = m_DepthStencil.back;
            m_DepthStencil.back.compareOp = VK_COMPARE_OP_ALWAYS;
        }

        void Builder::EnableDepthTest()
        {
            m_DepthStencil.depthTestEnable = VK_TRUE;
            m_DepthStencil.depthWriteEnable = VK_TRUE;
            m_DepthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
            m_DepthStencil.front = m_DepthStencil.back;
            m_DepthStencil.back.compareOp = VK_COMPARE_OP_ALWAYS;
        }

        void Builder::EnableStencilTest()
        {
            m_DepthStencil.stencilTestEnable = VK_TRUE;
            m_DepthStencil.back.compareOp = VK_COMPARE_OP_NOT_EQUAL;

            m_DepthStencil.back.passOp = VK_STENCIL_OP_REPLACE;
            m_DepthStencil.back.failOp = VK_STENCIL_OP_KEEP;
            m_DepthStencil.back.depthFailOp = VK_STENCIL_OP_KEEP;

            m_DepthStencil.back.compareMask = 0xff;
            m_DepthStencil.back.writeMask = 0;
            m_DepthStencil.back.reference = 1;
            m_DepthStencil.front = m_DepthStencil.back;
        }

        void Builder::EnableStencilWrite()
        {
            m_DepthStencil.stencilTestEnable = VK_TRUE;
            m_DepthStencil.back.compareOp = VK_COMPARE_OP_ALWAYS;

            m_DepthStencil.back.passOp = VK_STENCIL_OP_REPLACE;
            m_DepthStencil.back.failOp = VK_STENCIL_OP_REPLACE;
            m_DepthStencil.back.depthFailOp = VK_STENCIL_OP_REPLACE;

            m_DepthStencil.back.compareMask = 0;
            m_DepthStencil.back.writeMask = 0xff;
            m_DepthStencil.back.reference = 2;
            m_DepthStencil.front = m_DepthStencil.back;
        }

        void Builder::SetVertexInput(VkVertexInputBindingDescription bindingDescription, const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
        {
            m_VertexBindingDescription = bindingDescription;
            m_VertexAttributeDescriptions = std::move(attributeDescriptions);

            m_VertexInput.vertexBindingDescriptionCount = 1;
            m_VertexInput.pVertexBindingDescriptions = &m_VertexBindingDescription;

            m_VertexInput.vertexAttributeDescriptionCount = static_cast<DkU32>(m_VertexAttributeDescriptions.size());
            m_VertexInput.pVertexAttributeDescriptions = m_VertexAttributeDescriptions.data();
        }

        void Builder::Clear()
        {
            m_InputAssembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };

            m_Rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };

            m_Multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };

            m_ColorBlend = { .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };

            m_ColorBlendAttachment = {};

            m_DepthStencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };

            m_VertexInput = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

            m_VertexBindingDescription = {};
            m_VertexAttributeDescriptions.clear();

            m_ShaderStages.clear();
        }

    }
}
