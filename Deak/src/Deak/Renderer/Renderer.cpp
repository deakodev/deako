#include "Renderer.h"
#include "dkpch.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Platform/OpenGL/OpenGLShader.h"

namespace Deak {

    Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData;

    void Renderer::BeginScene(Camera& camera)
    {
        s_SceneData->ViewProjection = camera.GetViewProjection();
    }

    void Renderer::EndScene()
    {
    }

    void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& model)
    {
        shader->Bind();
        std::dynamic_pointer_cast<OpenGLShader>(shader)->setUniformMat4("u_ViewProjection", s_SceneData->ViewProjection);
        std::dynamic_pointer_cast<OpenGLShader>(shader)->setUniformMat4("u_Model", model);

        vertexArray->Bind();
        RenderCommand::DrawArrays(vertexArray);
    }

}

