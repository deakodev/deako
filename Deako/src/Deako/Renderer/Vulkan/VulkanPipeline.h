#pragma once

#include <vulkan/vulkan.h>

namespace Deako {
    namespace VulkanPipeline {

        class Builder
        {
        public:
            Builder() { Clear(); }

            VkPipeline Build(VkPipelineLayout pipelineLayout, const VkFormat* colorAttachmentFormats);
            void Clear();

            void SetShaders(VkShaderModule vertexModule, VkShaderModule fragmentModule);

            void SetInputTopology(VkPrimitiveTopology topology);
            void SetPolygonMode(VkPolygonMode mode);
            void SetCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace);
            void SetMultisampling(VkSampleCountFlagBits sampleFlag);
            void DisableColorBlending();
            void DisableDepthTest();
            void EnableDepthTest();
            void EnableAlphaBlending();
            void SetVertexInput(VkVertexInputBindingDescription bindingDescription, const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions);

        private:
            std::vector<VkPipelineShaderStageCreateInfo>   m_ShaderStages;
            VkPipelineInputAssemblyStateCreateInfo         m_InputAssembly;
            VkPipelineRasterizationStateCreateInfo         m_Rasterizer;
            VkPipelineMultisampleStateCreateInfo           m_Multisampling;
            VkPipelineColorBlendStateCreateInfo            m_ColorBlend;
            VkPipelineColorBlendAttachmentState            m_ColorBlendAttachment;
            VkPipelineDepthStencilStateCreateInfo          m_DepthStencil;
            VkPipelineVertexInputStateCreateInfo           m_VertexInput;
            VkVertexInputBindingDescription                m_VertexBindingDescription;
            std::vector<VkVertexInputAttributeDescription> m_VertexAttributeDescriptions;

        };

    }
}
