#include "VulkanPipeline.h"
#include "dkpch.h"

#include "VulkanBase.h"
#include "VulkanDevice.h"
#include "VulkanShaderModule.h"
#include "VulkanRenderPass.h"
#include "VulkanBuffer.h"

namespace Deako {

    VkPipeline VulkanPipeline::s_GraphicsPipeline;
    VkPipelineLayout VulkanPipeline::s_PipelineLayout;

    void VulkanPipeline::Create()
    {
        VkDevice device = VulkanDevice::GetLogical();

        VkShaderModule vertShaderModule = ShaderModule::Create(
            "/Users/deakzach/Desktop/Deako/Deako-Editor/assets/shaders/bin/shader.vert.spv");
        VkShaderModule fragShaderModule = ShaderModule::Create(
            "/Users/deakzach/Desktop/Deako/Deako-Editor/assets/shaders/bin/shader.frag.spv");

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main"; // Entry point, can potentially link multiple shaders into a module
        // vertShaderStageInfo.pSpecializationInfo - allows for specifying shader constants. Auto set to nullptr

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        //--- Vertex input ---//
        // describes the format of the vertex data that will be passed to the vertex shader:
        //  • Bindings: spacing between data and whether the data is per-vertex or per-instance (see instancing)
        //  • Attribute descriptions: type of attributes passed to the vertex shader, which binding to load them from and at which offset
        auto bindingDescription = Vertex::GetBindingDescription();
        auto attributeDescriptions = Vertex::GetAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        //--- Input assembly ---//
        // describes what kind of geometry will be drawn from vertices and if primitive restart should be enabled
        //  • VK_PRIMITIVE_TOPOLOGY_POINT_LIST: points from vertices
        //  • VK_PRIMITIVE_TOPOLOGY_LINE_LIST: line from every 2 vertices without reuse
        //  • VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: end vertex of every line is used as start vertex for next line
        //  • VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: triangle from every 3 vertices without reuse
        //  • VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: the second and third vertex of every triangle are used as first two vertices of the next triangle
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        //--- Dynamic states ---//
        // *some* pipeline states can be changed without recreating the pipeline at draw time. Eg. size of the viewport, line width and blend constants
        std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        //--- Rasterizer ---//
        // takes the geometry that is shaped by the vertices from vertex shader and turns it into fragments to be colored by the fragment shader
        //  • also performs depth testing, face culling, and scissor test, and it can be configured for wireframe rendering
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        // if VK_TRUE, fragments beyond near/far planes are clamped to them as opposed to discarding them, useful for shadow maps, requires enabling a GPU feature
        rasterizer.depthClampEnable = VK_FALSE;
        // if VK_TRUE, then geometry never passes through the rasterizer stage, basically disables any output to the framebuffer
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        // determines how fragments are generated for geometry:
        // • VK_POLYGON_MODE_FILL: fill the area of the polygon with fragments
        // • VK_POLYGON_MODE_LINE: polygon edges are drawn as lines
        // • VK_POLYGON_MODE_POINT: polygon vertices are drawn as points
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // any mode requires enabling a GPU feature
        // thickness of lines in terms of number of fragments, max line width depends on the hardware
        rasterizer.lineWidth = 1.0f; // line thicker than 1.0f requires enabling wideLines GPU feature

        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        // these are sometimes used for shadow mapping
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional 
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        //--- Multisampling ---//
        // one of the ways to perform anti-aliasing, works by combining fragment shader results of multiple polygons that rasterize to the same pixel (eg. along edges)
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE; // enabling requires enabling a GPU feature
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        //--- Depth and stencil testing ---//
        // TODO: Will come back to this and multisampling in later chapters

        //--- Color blending ---//
        // after fragment shader returns color, needs to be combined with the color in the framebuffer
        // Two ways to do it:
        // • Mix the old and new value to produce a final color
        // • Combine the old and new value using a bitwise operation
        // first struct contains the configuration per attached framebuffer
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
            | VK_COLOR_COMPONENT_G_BIT
            | VK_COLOR_COMPONENT_B_BIT
            | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        // second struct contains the *global* color blending settings
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        // to use the second method of blending (bitwise combination), set logicOpEnable VK_TRUE. The bitwise operation can then be specified in the logicOp field, will automatically disable the first method
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional 
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        //--- Pipeline layout ---//
        // uniforms specified below
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &VulkanBufferPool::GetDescriptorSetLayout(); // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &s_PipelineLayout);
        DK_CORE_ASSERT(!result, "Failed to create pipeline layout!");

        //--- One struct to rule them all ---//
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        // Fixed functionality structs
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr; // Optional
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;

        pipelineInfo.layout = s_PipelineLayout;

        pipelineInfo.renderPass = VulkanRenderPass::GetRenderPass();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &s_GraphicsPipeline);
        DK_CORE_ASSERT(!result, "Failed to create graphics pipeline!");

        // Clean up shaders
        ShaderModule::CleanUp(vertShaderModule);
        ShaderModule::CleanUp(fragShaderModule);
    }

    void VulkanPipeline::CleanUp()
    {
        VkDevice device = VulkanDevice::GetLogical();
        vkDestroyPipeline(device, s_GraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, s_PipelineLayout, nullptr);
    }

}
