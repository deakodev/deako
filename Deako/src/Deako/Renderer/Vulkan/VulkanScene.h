#pragma once

#include "VulkanBase.h"

#include "Deako/Scene/Scene.h"
#include "Deako/Scene/Entity.h"

#include <vulkan/vulkan.h>

namespace Deako {

    class VulkanScene
    {
    public:
        static void Build();
        static void Rebuild();
        static void CleanUp();

        static void Draw(VkCommandBuffer commandBuffer, uint32_t imageIndex);

        static void UpdateUniforms();
        static void UpdateShaderParams();
        static void ViewportResize(const glm::vec2& viewportSize);

        static bool IsInvalid() { return !m_SceneValid; }
        static void Invalidate() { m_SceneValid = false; };

    private:
        static void SetUpAssets();
        static void SetUpUniforms();
        static void SetUpDescriptors();
        static void SetUpPipelines();

        static void AddPipelineSet(const std::string prefix, std::filesystem::path vertexShader, std::filesystem::path fragmentShader);

    private:
        inline static bool m_SceneValid{ false };
    };

}
