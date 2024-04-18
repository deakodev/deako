#pragma once

#include "Camera/Camera.h"
#include "Camera/OrthographicCameraController.h"
#include "Camera/PerspectiveCameraController.h"
#include "Texture.h"

namespace Deak {

    class Renderer2D
    {
    public:
        static void Init();
        static void Shutdown();

        //temp
        static void BeginScene(const Camera& camera);
        static void BeginScene(const OrthographicCameraController& cameraController);
        static void BeginScene(const PerspectiveCameraController& cameraController);

        static void Flush();

        static void SetVBPointers();
        static void SetIndexCounts();

        static void DrawTriangle(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
        static void DrawTriangle(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float textureScalar, const glm::vec4 textureTint);

        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float textureScalar, const glm::vec4 textureTint);
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float textureScalar, const glm::vec4 textureTint);
    };

}
