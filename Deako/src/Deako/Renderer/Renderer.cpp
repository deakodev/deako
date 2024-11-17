#include "Renderer.h"
#include "dkpch.h"

#include "Deako/Project/ProjectHandler.h"
#include "Deako/Asset/AssetManager.h"
#include "Deako/Renderer/Vulkan/VulkanBase.h"
#include "Deako/Renderer/Vulkan/VulkanPicker.h"
#include "Deako/Renderer/Vulkan/VulkanScene.h"
#include "Deako/Renderer/Vulkan/VulkanStats.h"

namespace Deako {

    void Renderer::Init()
    {
        VulkanBase::Init();

        VulkanPicker::Init();

        ProjectHandler::Init();

        AssetManager::Init();

        VulkanScene::Build();
    }

    void Renderer::Shutdown()
    {
        VulkanScene::CleanUp();

        AssetManager::CleanUp();

        ProjectHandler::CleanUp();

        VulkanPicker::CleanUp();

        VulkanBase::Shutdown();
    }

    void Renderer::Render()
    {
        ClearStats();

        if (!GetActiveScene().isValid)
        {
            VulkanScene::Rebuild();
            return;
        }

        VulkanBase::Draw();

        QueryStats();
    }

    void Renderer::ClearStats()
    {
        s_SceneStats.primitiveCount = 0;
        s_SceneStats.drawCallCount = 0;
    }

    void Renderer::QueryStats()
    {
        s_SceneStats.frameTime = VulkanStats::QueryTimestamps();
    }

}
