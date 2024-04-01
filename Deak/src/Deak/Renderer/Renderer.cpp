#include "Renderer.h"
#include "dkpch.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Deak {

    Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData;

    void Renderer::BeginScene(Camera& camera)
    {
        s_SceneData->ViewProjection = camera.GetViewProjection();
    }

    void Renderer::EndScene()
    {
    }

    void Renderer::Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray)
    {
        shader->Bind();
        shader->setMat4("u_ViewProjection", s_SceneData->ViewProjection);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, 20.0f, glm::vec3(0.5f, 1.0f, 0.0f));
        shader->setMat4("u_Model", model);

        vertexArray->Bind();
        RenderCommand::DrawArrays(vertexArray);
    }

}

