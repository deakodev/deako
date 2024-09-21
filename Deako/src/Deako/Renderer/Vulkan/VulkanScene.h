#pragma once

#include "VulkanBase.h"

#include <vulkan/vulkan.h>

namespace Deako {

    class VulkanScene
    {
    public:
        static void Prepare();
        static void Render();
        static void CleanUp();

        static void SetPrepared(bool prepared);

        static void ViewportResize(const glm::vec2& viewportSize);

    private:
        static void SetUpAssets();
        static void SetUpUniforms();
        static void SetUpDescriptors();
        static void SetUpPipelines();

        static void AddPipelineSet(const std::string prefix, std::filesystem::path vertexShader, std::filesystem::path fragmentShader);

        static void Draw();
        static void DrawViewport(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        static void DrawImGui(VkCommandBuffer commandBuffer, uint32_t imageIndex);

        static void UpdateUniforms();
        static void UpdateShaderParams();
    };

}
