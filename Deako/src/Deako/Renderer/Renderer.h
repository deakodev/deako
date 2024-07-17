#pragma once

#include "System/Vulkan/VulkanBuffer.h"

namespace Deako {

    class Renderer
    {
    public:
        static void Init(const char* appName);
        static void Shutdown();

        static void BeginScene(const glm::mat4 viewProjection);
        static void EndScene();
        static void NextBatch();

    private:
        static void Flush();
        static void StartBatch();
    };

}
