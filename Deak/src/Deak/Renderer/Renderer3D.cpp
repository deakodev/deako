#include "Renderer3D.h"
#include "dkpch.h"

#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Deak {

    struct Renderer3DData
    {
        Ref<VertexArray> vertexArray;
        Ref<Shader> textureShader;
        Ref<Texture2D> whiteTexture;
    };

    static Renderer3DData s_3DData;

    void Renderer3D::Init()
    {
        s_3DData.vertexArray = VertexArray::Create();

        float vertices[] = {
            // Back face
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
             0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
             0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
            // Front face
            -0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
             0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
             0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f, 0.0f, 1.0f,
            // Left face
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f, 1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
            -0.5f, -0.5f,  0.5f, 0.0f, 1.0f,
            // Right face
             0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
             0.5f,  0.5f, -0.5f, 1.0f, 0.0f,
             0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
             0.5f, -0.5f,  0.5f, 0.0f, 1.0f,
             // Top face
             -0.5f,  0.5f, -0.5f, 0.0f, 0.0f,
              0.5f,  0.5f, -0.5f, 1.0f, 0.0f,
              0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
             -0.5f,  0.5f,  0.5f, 0.0f, 1.0f,
             // Bottom face
             -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
              0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
              0.5f, -0.5f,  0.5f, 1.0f, 1.0f,
             -0.5f, -0.5f,  0.5f, 0.0f, 1.0f,
        };

        Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
        vertexBuffer->SetLayout({
           { ShaderDataType::Float3, "a_Position" },
           { ShaderDataType::Float2, "a_TexCoord" }
            });
        s_3DData.vertexArray->AddVertexBuffer(vertexBuffer);

        uint32_t indices[] = {
            // Back face
            0, 1, 2, 2, 3, 0,
            // Front face
            4, 5, 6, 6, 7, 4,
            // Left face
            8, 9, 10, 10, 11, 8,
            // Right face
            12, 13, 14, 14, 15, 12,
            // Top face
            16, 17, 18, 18, 19, 16,
            // Bottom face
            20, 21, 22, 22, 23, 20
        };

        Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
        s_3DData.vertexArray->AddIndexBuffer(indexBuffer);

        s_3DData.whiteTexture = Texture2D::Create(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        s_3DData.whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

        s_3DData.textureShader = Shader::Create("Sandbox/assets/shaders/Example3D.glsl");
        s_3DData.textureShader->Bind();
        s_3DData.textureShader->SetInt("u_Texture", 0);
    }

    void Renderer3D::Shutdown()
    {
    }

    void Renderer3D::BeginScene(const PerspectiveCamera& camera)
    {
        s_3DData.textureShader->Bind();
        s_3DData.textureShader->SetMat4("u_ViewProjection", camera.GetViewProjection());
    }

    void Renderer3D::EndScene()
    {
    }

    void Renderer3D::DrawQuad(const glm::vec2& position, const glm::vec3& size, const glm::vec4& color)
    {
        DrawQuad({ position.x, position.y, 0.0f }, size, color);
    }

    void Renderer3D::DrawQuad(const glm::vec3& position, const glm::vec3& size, const glm::vec4& color)
    {
        s_3DData.textureShader->SetFloat4("u_Color", color);
        s_3DData.whiteTexture->Bind();

        glm::mat4 model = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 1.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), { size.x, size.y, size.z });
        s_3DData.textureShader->SetMat4("u_Model", model);

        s_3DData.vertexArray->Bind();
        RenderCommand::DrawIndexed(s_3DData.vertexArray);
    }

    void Renderer3D::DrawQuad(const glm::vec2& position, const glm::vec3& size, const Ref<Texture2D>& texture)
    {
        DrawQuad({ position.x, position.y, 0.0f }, size, texture);
    }

    void Renderer3D::DrawQuad(const glm::vec3& position, const glm::vec3& size, const Ref<Texture2D>& texture)
    {
        s_3DData.textureShader->SetFloat4("u_Color", glm::vec4(1.0f));
        texture->Bind();

        glm::mat4 model = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 1.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), { size.x, size.y, size.z });
        s_3DData.textureShader->SetMat4("u_Model", model);

        s_3DData.vertexArray->Bind();
        RenderCommand::DrawIndexed(s_3DData.vertexArray);
    }

}
