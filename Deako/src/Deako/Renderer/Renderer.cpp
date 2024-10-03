#include "Renderer.h"
#include "dkpch.h"

#include "Deako/Project/Project.h"
#include "Deako/Asset/Pool/AssetManager.h"
#include "Deako/Renderer/Vulkan/VulkanBase.h"
#include "Deako/Renderer/Vulkan/VulkanScene.h"

namespace Deako {

    void Renderer::Init()
    {
        VulkanBase::Init();

        Project::Init();

        AssetManager::Init();

        Project::PrepareScene();

        VulkanScene::Build();
    }

    void Renderer::Shutdown()
    {
        VulkanScene::CleanUp();

        AssetManager::CleanUp();

        Project::CleanUp();

        VulkanBase::Shutdown();
    }

    void Renderer::BeginScene()
    {
    }

    void Renderer::EndScene()
    {
        VulkanBase::Render();
    }

    void Renderer::Invalidate()
    {
        VulkanScene::Invalidate();
    }

}
