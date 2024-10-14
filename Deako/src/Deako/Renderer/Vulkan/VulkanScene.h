#pragma once

#include "VulkanBase.h"

#include "Deako/Asset/AssetManager.h"
#include "Deako/Asset/Pool/ProjectAssetPool.h"
#include "Deako/Asset/Scene/Scene.h"
#include "Deako/Asset/Scene/Entity.h"

#include "Deako/Renderer/EditorCamera.h"

#include <vulkan/vulkan.h>

namespace Deako {

    class VulkanScene
    {
    public:
        static void Build();
        static void Rebuild();
        static void CleanUp();

        static void Draw(VkCommandBuffer commandBuffer, uint32_t imageIndex);

        static void UpdateUniforms(Ref<EditorCamera> camera);

        static bool IsInvalid() { return !s_SceneValid; }
        static void Invalidate() { s_SceneValid = false; };

    private:
        static void SetUpAssets();
        static void SetUpUniforms();
        static void SetUpDescriptors();
        static void SetUpPipelines();

        static void AddPipelineSet(const std::string prefix, std::filesystem::path vertexShader, std::filesystem::path fragmentShader);

    private:
        inline static bool s_SceneValid{ false };

        inline static Ref<ProjectAssetPool> s_ProjectAssetPool;
    };

}
