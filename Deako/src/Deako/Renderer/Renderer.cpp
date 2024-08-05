#include "Renderer.h"
#include "dkpch.h"

#include "System/Vulkan/VulkanBase.h"

namespace Deako {

    uint16_t INSTANCE_COUNT = 0;

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
        VulkanBase::RenderFrame();
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
