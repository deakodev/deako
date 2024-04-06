#include "Renderer2D.h"
#include "dkpch.h"

#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Deak {

    struct Renderer2DStorage
    {
        Ref<VertexArray> vertexArray;
        Ref<Shader> textureShader;
        Ref<Texture2D> whiteTexture;
    };

    static Renderer2DStorage* s_Data;

    void Renderer2D::Init()
    {
        s_Data = new Renderer2DStorage();

        s_Data->vertexArray = VertexArray::Create();

        float vertices[] = {
                -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
                 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
                 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
                -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
        };

        Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
        vertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float2, "a_TexCoord" }
            });
        s_Data->vertexArray->AddVertexBuffer(vertexBuffer);

        uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };
        Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
        s_Data->vertexArray->SetIndexBuffer(indexBuffer);

        s_Data->whiteTexture = Texture2D::Create(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        s_Data->whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

        s_Data->textureShader = Shader::Create("Sandbox/assets/shaders/Example2D.glsl");
        s_Data->textureShader->Bind();
        s_Data->textureShader->SetInt("u_Texture", 0);
    }

    void Renderer2D::Shutdown()
    {
        delete s_Data;
    }

    void Renderer2D::BeginScene(const OrthographicCamera& camera)
    {
        s_Data->textureShader->Bind();
        s_Data->textureShader->SetMat4("u_ViewProjection", camera.GetViewProjection());
    }

    void Renderer2D::EndScene()
    {
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        DrawQuad({ position.x, position.y, 0.0f }, size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
    {
        s_Data->textureShader->SetFloat4("u_Color", color);
        s_Data->whiteTexture->Bind();

        glm::mat4 model = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
        s_Data->textureShader->SetMat4("u_Model", model);

        s_Data->vertexArray->Bind();
        RenderCommand::DrawIndexed(s_Data->vertexArray);
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture)
    {
        DrawQuad({ position.x, position.y, 0.0f }, size, texture);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture)
    {
        s_Data->textureShader->SetFloat4("u_Color", glm::vec4(1.0f));
        texture->Bind();

        glm::mat4 model = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
        s_Data->textureShader->SetMat4("u_Model", model);

        s_Data->vertexArray->Bind();
        RenderCommand::DrawIndexed(s_Data->vertexArray);
    }



}
