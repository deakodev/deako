#include "Renderer.h"
#include "dkpch.h"

#include "Deako/Asset/AssetPool.h"
#include "Deako/Renderer/Vulkan/VulkanBase.h"
#include "Deako/Renderer/Vulkan/VulkanScene.h"

namespace Deako {

    void Renderer::Init()
    {
        VulkanBase::Init();

        Scene::LinkAssets(); // TODO: figure out where to do this

        VulkanScene::Build();
    }

    void Renderer::Shutdown()
    {
        VulkanScene::CleanUp();

        AssetPool::CleanUp();

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
