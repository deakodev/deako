#pragma once

// #include "System/Vulkan/VulkanBuffer.h"

#include <glm/glm.hpp>

namespace Deako {

    const uint16_t MAX_INSTANCE_COUNT = 2;
    extern uint16_t INSTANCE_COUNT;
    const uint32_t MAX_VERTICES = 120000;

    struct RendererData
    {
        // uint32_t batchVerticesCount;
        // uint32_t batchIndicesCount;

        // std::vector<InstanceData> modelInstanceData;
        // Ref<InstanceBuffer> modelInstanceBuffer;
        // Ref<VertexBuffer> modelVertexBuffer;
        // Ref<IndexBuffer> modelIndexBuffer;
    };

    class Renderer
    {
    public:
        static void Init(const char* appName);
        static void Shutdown();

        static void BeginScene();
        static void BeginScene(const glm::mat4& viewProjection);
        static void EndScene();
        static void NextBatch();

        // static void PrepareModelInstance(const glm::vec3& position, const glm::vec3& rotation, float scale, uint32_t textureIndex);

        // static const Ref<VertexBuffer>& GetModelVertexBuffer() { return s_Data.modelVertexBuffer; }
        // static const Ref<IndexBuffer>& GetModelIndexBuffer() { return s_Data.modelIndexBuffer; }
        // static const Ref<InstanceBuffer>& GetModelInstanceBuffer() { return s_Data.modelInstanceBuffer; }

    private:
        static void Flush();
        static void StartBatch();

    private:
        static RendererData s_Data;
    };

}
