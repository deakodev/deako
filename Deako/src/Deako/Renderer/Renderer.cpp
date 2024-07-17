#include "Renderer.h"
#include "dkpch.h"

#include "System/Vulkan/VulkanBase.h"
#include "System/Vulkan/VulkanBuffer.h"

namespace Deako {

    void Renderer::Init(const char* appName)
    {
        VulkanBase::Init(appName);
    }

    void Renderer::Shutdown()
    {
        VulkanBase::Idle();
        VulkanBase::Shutdown();
    }

    void Renderer::BeginScene(const glm::mat4 viewProjection)
    {
        StartBatch();
        // BufferPool::UpdateUniformBuffer(viewProjection);
        VulkanBase::DrawFrame(viewProjection);
    }

    void Renderer::EndScene()
    {
        Flush();
    }

    void Renderer::NextBatch()
    {
        Flush();
        StartBatch();
    }

    void Renderer::Flush()
    {
        //....
        // VulkanBase::DrawFrame();
    }

    void Renderer::StartBatch()
    {

    }

}
