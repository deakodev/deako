#include "Renderer.h"
#include "dkpch.h"

#include "Renderer2D.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Deak {

    Scope<Renderer::SceneData> Renderer::s_SceneData = CreateScope<Renderer::SceneData>();

    void Renderer::Init()
    {
        RenderCommand::Init();
        Renderer2D::Init();
    }

    void Renderer::OnWindowResize(uint32_t width, uint32_t height)
    {
        RenderCommand::SetViewport(0, 0, width, height);
    }

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
        std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_ViewProjection", s_SceneData->ViewProjection);
        std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_Model", model);

        vertexArray->Bind();
        RenderCommand::DrawArrays(vertexArray);
    }

}

