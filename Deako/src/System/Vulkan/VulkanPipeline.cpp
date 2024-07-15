#include "VulkanPipeline.h"
#include "dkpch.h"

#include "VulkanShaderModule.h"
#include "VulkanBuffer.h"

namespace Deako {

    Ref<VulkanResources> Pipeline::s_VR = VulkanBase::GetResources();

    void Pipeline::Create()
    {
        //--- Shader stages ---//
        VkShaderModule vertShaderModule = ShaderModule::Create(
            "Deako-Editor/assets/shaders/bin/shader.vert.spv");
        VkShaderModule fragShaderModule = ShaderModule::Create(
            "Deako-Editor/assets/shaders/bin/shader.frag.spv");

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
        auto bindingDescriptions = Vertex::GetBindingDescription();
        auto attributeDescriptions = Vertex::GetAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo =
            VulkanInitializers::PipelineVertexInputStateCreateInfo(bindingDescriptions, attributeDescriptions);

        //--- Input assembly ---//
        // describes what kind of geometry will be drawn from vertices and if primitive restart should be enabled
        //  • VK_PRIMITIVE_TOPOLOGY_POINT_LIST: points from vertices
        //  • VK_PRIMITIVE_TOPOLOGY_LINE_LIST: line from every 2 vertices without reuse
        //  • VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: end vertex of every line is used as start vertex for next line
        //  • VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: triangle from every 3 vertices without reuse
        //  • VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: the second and third vertex of every triangle are used as first two vertices of the next triangle
        VkPipelineInputAssemblyStateCreateInfo inputAssembly =
            VulkanInitializers::PipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

        //--- Dynamic states ---//
        // *some* pipeline states can be changed without recreating the pipeline at draw time. Eg. size of the viewport, line width and blend constants
        std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH };

        VkPipelineDynamicStateCreateInfo dynamicState =
            VulkanInitializers::PipelineDynamicStateCreateInfo(dynamicStates);

        VkViewport viewport =
            VulkanInitializers::Viewport((float)s_VR->imageExtent.width, (float)s_VR->imageExtent.height, 0.0f, 1.0f);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = s_VR->imageExtent;

        VkPipelineViewportStateCreateInfo viewportState =
            VulkanInitializers::PipelineViewportStateCreateInfo();
        viewportState.pViewports = &viewport;
        viewportState.pScissors = &scissor;

        //--- Rasterizer ---//
        // takes the geometry that is shaped by the vertices from vertex shader and turns it into fragments to be colored by the fragment shader
        //  • also performs depth testing, face culling, and scissor test, and it can be configured for wireframe rendering
        VkPipelineRasterizationStateCreateInfo rasterizer =
            VulkanInitializers::PipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        // if VK_TRUE, then geometry never passes through the rasterizer stage, basically disables any output to the framebuffer
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        // these are sometimes used for shadow mapping
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional 
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        //--- Multisampling ---//
        // one of the ways to perform anti-aliasing, works by combining fragment shader results of multiple polygons that rasterize to the same pixel (eg. along edges)
        VkPipelineMultisampleStateCreateInfo multisampling =
            VulkanInitializers::PipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);
        multisampling.sampleShadingEnable = VK_FALSE; // enabling requires enabling a GPU feature
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        //--- Depth and stencil testing ---//
        VkPipelineDepthStencilStateCreateInfo depthStencil =
            VulkanInitializers::PipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS);
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; // Optional
        depthStencil.back = {};	 // Optional

        //--- Color blending ---//
        // after fragment shader returns color, needs to be combined with the color in the framebuffer
        // Two ways to do it:
        // • Mix the old and new value to produce a final color
        // • Combine the old and new value using a bitwise operation
        // first struct contains the configuration per attached framebuffer
        VkPipelineColorBlendAttachmentState colorBlendAttachment =
            VulkanInitializers::PipelineColorBlendAttachmentState(
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT, VK_FALSE);
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        // second struct contains the *global* color blending settings
        VkPipelineColorBlendStateCreateInfo colorBlending =
            VulkanInitializers::PipelineColorBlendStateCreateInfo(1, &colorBlendAttachment);
        // to use the second method of blending (bitwise combination), set logicOpEnable VK_TRUE. The bitwise operation can then be specified in the logicOp field, will automatically disable the first method
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional 
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        //--- Pipeline layout ---//
        VkDescriptorSetLayout setLayouts[1] = { s_VR->descriptorSetLayout };
        VkPipelineLayoutCreateInfo pipelineLayoutInfo =
            VulkanInitializers::PipelineLayoutCreateInfo(setLayouts, 1);
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        VkResult result = vkCreatePipelineLayout(s_VR->device, &pipelineLayoutInfo, nullptr, &s_VR->pipelineLayout);
        DK_CORE_ASSERT(!result);

        //--- One struct to rule them all ---//
        VkGraphicsPipelineCreateInfo pipelineInfo =
            VulkanInitializers::PipelineCreateInfo(s_VR->pipelineLayout, s_VR->renderPass);
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        // Fixed functionality structs
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil; // Optional
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.subpass = 0;

        result = vkCreateGraphicsPipelines(s_VR->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &s_VR->graphicsPipeline);
        DK_CORE_ASSERT(!result);

        // Viewport pipeline
        pipelineInfo.renderPass = s_VR->viewportRenderPass;
        result = vkCreateGraphicsPipelines(s_VR->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &s_VR->viewportPipeline);
        DK_CORE_ASSERT(!result);

        // Clean up shaders
        ShaderModule::CleanUp(vertShaderModule);
        ShaderModule::CleanUp(fragShaderModule);
    }

    void Pipeline::CleanUp()
    {
        vkDestroyPipeline(s_VR->device, s_VR->viewportPipeline, nullptr);
        vkDestroyPipeline(s_VR->device, s_VR->graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(s_VR->device, s_VR->pipelineLayout, nullptr);
    }

}
