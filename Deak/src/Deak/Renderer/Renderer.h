#pragma once

#include "RenderCommand.h"
#include "Camera/Camera.h"
#include "Shader.h"

namespace Deak {

    class Renderer
    {
    public:
        static void Init();

        static void BeginScene(Camera& camera);
        static void EndScene();

        static void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& model = glm::mat4(1.0f));


        inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

    private:
        struct SceneData
        {
            glm::mat4 ViewProjection;
        };

        static SceneData* s_SceneData;

    };

}
