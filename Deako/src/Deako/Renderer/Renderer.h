#pragma once

// #include "Deako/Renderer/Vulkan/VulkanBuffer.h"

#include <glm/glm.hpp>

namespace Deako {

    const uint16_t MAX_INSTANCE_COUNT = 2;
    extern uint16_t INSTANCE_COUNT;
    const uint32_t MAX_VERTICES = 120000;

    struct RendererData
    {

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

    private:
        static void Flush();
        static void StartBatch();

    private:
        static RendererData s_Data;
    };

}
