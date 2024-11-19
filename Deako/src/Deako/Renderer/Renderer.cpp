#include "Renderer.h"
#include "dkpch.h"

#include "Deako/Project/ProjectHandler.h"
#include "Deako/Asset/AssetManager.h"

namespace Deako {

    void Renderer::Init()
    {
        VulkanBase::Init();

        ProjectHandler::Init();

        AssetManager::Init();

        Scene& activeScene = GetActiveScene();
        activeScene.Build();
    }

    void Renderer::Shutdown()
    {
        Scene& activeScene = GetActiveScene();
        activeScene.CleanUp();

        AssetManager::CleanUp();

        ProjectHandler::CleanUp();

        VulkanBase::Shutdown();
    }

    void Renderer::Render()
    {
        ClearStats();

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
