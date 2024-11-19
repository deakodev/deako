#pragma once

#include "Deako/Renderer/EditorCamera.h"
#include "Deako/Renderer/Vulkan/VulkanBase.h"
#include "Deako/Renderer/Vulkan/VulkanScene.h"
#include "Deako/Renderer/Vulkan/VulkanPicker.h"
#include "Deako/Renderer/Vulkan/VulkanStats.h"

namespace Deako {

    struct RendererStats
    {
        DkF32 frameTime;
        DkU64 primitiveCount;
        DkU64 drawCallCount;
    };

    class Renderer
    {
    public:
        static void Init();
        static void Shutdown();

        static void Render();

        static void ClearStats();
        static void QueryStats();
        static RendererStats& GetSceneStats() { return s_SceneStats; };

    private:
        // inline static RendererStats s_EngineStats;
        inline static RendererStats s_SceneStats;
    };

}
