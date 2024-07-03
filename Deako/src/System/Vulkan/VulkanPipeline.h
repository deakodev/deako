#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

    class VulkanPipeline
    {
    public:
        static void Create();
        static void CleanUp();

        static VkPipeline GetPipeline() { return s_GraphicsPipeline; }
        static VkPipelineLayout GetPipelineLayout() { return s_PipelineLayout; }

    private:
        static VkPipeline s_GraphicsPipeline;
        static VkPipelineLayout s_PipelineLayout;
    };

}
