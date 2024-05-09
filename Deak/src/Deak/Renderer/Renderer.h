#pragma once

#include "Deak/Scene/SceneCamera.h"
#include "Camera/EditorCameraController.h"
#include "RendererAPI.h"
#include "RenderCommand.h"    
#include "Texture.h"
#include "UniformBuffer.h"

namespace Deak {

    struct RendererStats
    {
        uint32_t drawCalls = 0; // Total draw calls
        uint32_t primitiveCount = 0; // Total triangles, quads, etc.
        uint32_t vertexCount = 0; // Total vertices
        uint32_t indexCount = 0; // Total indices

        void AddPrimitive(uint32_t verticesPerPrimitive, uint32_t indicesPerPrimitive, uint32_t count = 1)
        {
            primitiveCount += count;
            vertexCount += verticesPerPrimitive;
            indexCount += indicesPerPrimitive;
        }
    };

    struct RendererData
    {
        static constexpr uint32_t MAX_VERTICES = 120000;
        static constexpr uint32_t MAX_INDICES = 120000;
        static constexpr uint32_t MAX_TEXTURE_SLOTS = 16; // 16 slots for MacOS

        uint32_t totalIndices = 0;

        Ref<Texture2D> defaultTexture; // default white texture
        int32_t textureSamplers[MAX_TEXTURE_SLOTS];
        std::array<Ref<Texture2D>, MAX_TEXTURE_SLOTS> textureSlots;
        uint32_t textureSlotIndex = 1; // 0 is the default white texture

        struct CameraData
        {
            glm::mat4 viewProjection;
        };

        CameraData cameraBuffer;
        Ref<UniformBuffer> cameraUniformBuffer;
    };

    class Renderer
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginScene(const SceneCamera& camera, const glm::mat4& transform);
        static void BeginScene(const EditorCameraController& cameraController);

        static void EndScene();
        static void NextBatch();

        static void ResetStats();
        static Ref<RendererStats> GetRendererStats() { return s_Stats; }
        static Ref<RendererData> GetRendererData() { return s_Data; }

        static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
        static void OnWindowResize(uint32_t width, uint32_t height) { RenderCommand::SetViewport(0, 0, width, height); }

    private:
        static void Flush();
        static void StartBatch();

        static void SetDefaultTextureData();

    private:
        static Ref<RendererStats> s_Stats;
        static Ref<RendererData> s_Data;
    };

}
