#include "Renderer.h"
#include "dkpch.h"

#include "Deako/Project/ProjectHandler.h"
#include "Deako/Asset/AssetManager.h"
#include "Deako/Renderer/Vulkan/VulkanBase.h"
#include "Deako/Renderer/Vulkan/VulkanPicker.h"
#include "Deako/Renderer/Vulkan/VulkanScene.h"

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
        VulkanBase::Draw();
    }

}
