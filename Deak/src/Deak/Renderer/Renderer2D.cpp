#include "Renderer2D.h"
#include "dkpch.h"

#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Deak {

    struct Renderer2DData
    {
        Ref<VertexArray> vertexArray;
        Ref<Shader> textureShader;
        Ref<Texture2D> whiteTexture;
    };

    static Renderer2DData s_2DData;

    void Renderer2D::Init()
    {
        DK_PROFILE_FUNC();

        s_2DData.vertexArray = VertexArray::Create();

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
        s_2DData.vertexArray->AddVertexBuffer(vertexBuffer);

        uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };
        Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
        s_2DData.vertexArray->AddIndexBuffer(indexBuffer);

        s_2DData.whiteTexture = Texture2D::Create(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        s_2DData.whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

        s_2DData.textureShader = Shader::Create("Sandbox/assets/shaders/Example2D.glsl");
        s_2DData.textureShader->Bind();
        s_2DData.textureShader->SetInt("u_Texture", 0);
    }

    void Renderer2D::Shutdown()
    {
        DK_PROFILE_FUNC();
    }

    void Renderer2D::BeginScene(const OrthographicCamera& camera)
    {
        DK_PROFILE_FUNC();

        s_2DData.textureShader->Bind();
        s_2DData.textureShader->SetMat4("u_ViewProjection", camera.GetViewProjection());
    }

    void Renderer2D::EndScene()
    {
        DK_PROFILE_FUNC();
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        DrawQuad({ position.x, position.y, 0.0f }, size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
    {
        DK_PROFILE_FUNC();

        s_2DData.textureShader->SetFloat4("u_Color", color);
        s_2DData.textureShader->SetFloat("u_TexScalar", 1.0f);
        s_2DData.whiteTexture->Bind();

        glm::mat4 model = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
        s_2DData.textureShader->SetMat4("u_Model", model);

        s_2DData.vertexArray->Bind();
        RenderCommand::DrawIndexed(s_2DData.vertexArray);
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float texScalar, const glm::vec4 texTint)
    {
        DrawQuad({ position.x, position.y, 0.0f }, size, texture, texScalar, texTint);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float texScalar, const glm::vec4 texTint)
    {
        DK_PROFILE_FUNC();

        s_2DData.textureShader->SetFloat4("u_Color", texTint);
        s_2DData.textureShader->SetFloat("u_TexScalar", texScalar);

        texture->Bind();

        glm::mat4 model = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
        s_2DData.textureShader->SetMat4("u_Model", model);

        s_2DData.vertexArray->Bind();
        RenderCommand::DrawIndexed(s_2DData.vertexArray);
    }

    void Renderer2D::DrawRotQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color)
    {
        DrawRotQuad({ position.x, position.y, 0.0f }, size, rotation, color);
    }

    void Renderer2D::DrawRotQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color)
    {
        DK_PROFILE_FUNC();

        s_2DData.textureShader->SetFloat4("u_Color", color);
        s_2DData.textureShader->SetFloat("u_TexScalar", 1.0f);
        s_2DData.whiteTexture->Bind();

        glm::mat4 model = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f, 0.0f, 1.0f))
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
        s_2DData.textureShader->SetMat4("u_Model", model);

        s_2DData.vertexArray->Bind();
        RenderCommand::DrawIndexed(s_2DData.vertexArray);
    }

    void Renderer2D::DrawRotQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float texScalar, const glm::vec4 texTint)
    {
        DrawRotQuad({ position.x, position.y, 0.0f }, size, rotation, texture, texScalar, texTint);
    }

    void Renderer2D::DrawRotQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float texScalar, const glm::vec4 texTint)
    {
        DK_PROFILE_FUNC();

        s_2DData.textureShader->SetFloat4("u_Color", texTint);
        s_2DData.textureShader->SetFloat("u_TexScalar", texScalar);
        texture->Bind();

        glm::mat4 model = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f, 0.0f, 1.0f))
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
        s_2DData.textureShader->SetMat4("u_Model", model);

        s_2DData.vertexArray->Bind();
        RenderCommand::DrawIndexed(s_2DData.vertexArray);
    }

}
