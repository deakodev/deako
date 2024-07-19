#include "Renderer.h"
#include "dkpch.h"

#include "System/Vulkan/VulkanBase.h"
#include "System/Vulkan/VulkanBuffer.h"

namespace Deako {

    uint16_t INSTANCE_COUNT = 0;

    RendererData Renderer::s_Data;

    void Renderer::Init(const char* appName)
    {
        VulkanBase::Init(appName);

        const std::vector<Vertex>& vertices = BufferPool::GetModel()->GetVertices();
        const std::vector<uint32_t>& indices = BufferPool::GetModel()->GetIndices();

        s_Data.modelVertexBuffer = CreateRef<VertexBuffer>(vertices);
        s_Data.modelIndexBuffer = CreateRef<IndexBuffer>(indices);

        PrepareModelInstance({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 0.75f, 0);
        PrepareModelInstance({ 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 0.5f, 0);
        s_Data.modelInstanceBuffer = CreateRef<InstanceBuffer>(s_Data.modelInstanceData);
    }

    void Renderer::Shutdown()
    {
        VulkanBase::Idle();

        s_Data.modelVertexBuffer->Destroy();
        s_Data.modelIndexBuffer->Destroy();
        s_Data.modelInstanceBuffer->Destroy();

        VulkanBase::Shutdown();
    }

    void Renderer::BeginScene(const glm::mat4 viewProjection)
    {
        StartBatch();
        BufferPool::UpdateUniformBuffer(viewProjection);
    }

    void Renderer::EndScene()
    {
        Flush();
    }

    void Renderer::Flush()
    {
        VulkanBase::DrawFrame();
    }

    void Renderer::StartBatch()
    {
    }

    void Renderer::NextBatch()
    {
        Flush();
        StartBatch();
    }

    void Renderer::PrepareModelInstance(const glm::vec3& position, const glm::vec3& rotation, float scale, uint32_t textureIndex)
    {
        s_Data.modelInstanceData.emplace_back(InstanceData{ position, rotation, scale, textureIndex });

        INSTANCE_COUNT++;
    }

}
