#pragma once

#include "Camera/PerspectiveCameraController.h"
#include "Texture.h"


namespace Deak {

    class Renderer3D
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginScene(const PerspectiveCameraController& cameraController, const glm::vec3& lightPosition);

        static void Flush();

        static void SetVBPointers();
        static void SetIndexCounts();

        static void DrawCube(const glm::vec3& position, const glm::vec3& size, const glm::vec4& color);
        static void DrawCube(const glm::vec3& position, const glm::vec3& size, const Ref<Texture2D>& texture, float textureScalar, const glm::vec4 textureTint);

        static void DrawCube(const glm::mat4& transform, const glm::vec4& color);
        static void DrawCube(const glm::mat4& transform, const Ref<Texture2D>& texture, float textureScalar, const glm::vec4 textureTint);

    };

}
