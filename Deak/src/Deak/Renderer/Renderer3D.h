#pragma once

#include "Camera/Camera.h"
#include "Texture.h"


namespace Deak {

    class Renderer3D
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginScene(const PerspectiveCamera& camera);
        static void EndScene();
        static void Flush();

        static void DrawCube(const glm::vec3& position, const glm::vec3& size, const glm::vec4& color);
        static void DrawCube(const glm::vec3& position, const glm::vec3& size, const Ref<Texture2D>& texture, float textureScalar = 1.0f, const glm::vec4 textureTint = glm::vec4(1.0f));

        struct Statistics
        {
            uint32_t drawCalls = 0;
            uint32_t cubeCount = 0;

            uint32_t GetTotalVertexCount() { return cubeCount * 24; }
            uint32_t GetTotalIndexCount() { return cubeCount * 36; }
        };

        static void ResetStats();
        static Statistics GetStats();

    private:
        static void FlushAndReset();
    };

}
