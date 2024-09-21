#include "Renderer.h"
#include "dkpch.h"

#include "Deako/Renderer/Vulkan/VulkanBase.h"

namespace Deako {

    RendererData Renderer::s_Data;

    void Renderer::Init(const char* appName)
    {
        VulkanBase::Init(appName);
    }

    void Renderer::Shutdown()
    {
        VulkanBase::Shutdown();
    }

    void Renderer::BeginScene()
    {

        StartBatch();
    }

    void Renderer::BeginScene(const glm::mat4& viewProjection)
    {
        StartBatch();
    }

    void Renderer::EndScene()
    {
        Flush();
    }

    void Renderer::Flush()
    {
        VulkanScene::Render();
    }

    void Renderer::StartBatch()
    {
    }

    void Renderer::NextBatch()
    {
        Flush();
        StartBatch();
    }

}
