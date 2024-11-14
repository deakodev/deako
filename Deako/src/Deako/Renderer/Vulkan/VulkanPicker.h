#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

    class VulkanPicker
    {
    public:
        static void Init();
        static void CleanUp();

        static void Draw(VkCommandBuffer commandBuffer, DkU32 imageIndex);

        static const DkVec4& GetPixelColor();

    private:
        static void SetUpTargets();
        // static void SetUpUniforms();
        // static void SetUpDescriptors();
        // static void SetUpPipelines();
    };

}
