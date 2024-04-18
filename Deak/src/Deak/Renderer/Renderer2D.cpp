#include "Renderer2D.h"
#include "dkpch.h"

#include "Renderer.h"
#include "VertexArray.h"
#include "Buffer.h"
#include "Shader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Deak {

    struct TriangleVertex
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 textureCoord;
        float textureIndex;
        float textureScalar;
    };

    struct QuadVertex
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 textureCoord;
        float textureIndex;
        float textureScalar;
    };

    struct PrimitiveData2D
    {
        // TRIANGLE
        Ref<VertexArray> triangleVA;
        Ref<VertexBuffer> triangleVB;
        Ref<Shader> triangleShader;
        TriangleVertex* triangleVBBase = nullptr;
        TriangleVertex* triangleVBPtr = nullptr;;
        glm::vec4 triangleVertPositions[3];
        uint32_t triangleIndexCount;

        // QUAD
        Ref<VertexArray> quadVA;
        Ref<VertexBuffer> quadVB;
        Ref<Shader> quadShader;
        QuadVertex* quadVBBase = nullptr;
        QuadVertex* quadVBPtr = nullptr;;
        glm::vec4 quadVertPositions[4];
        uint32_t quadIndexCount;
    };

    static PrimitiveData2D s_Data2D;
    static Ref<RendererData> s_RendererData;
    static Ref<RendererStats> s_RendererStats;

    void Renderer2D::Init()
    {
        DK_PROFILE_FUNC();

        s_RendererData = Renderer::GetRendererData();
        s_RendererStats = Renderer::GetRendererStats();

        s_Data2D.triangleVA = VertexArray::Create();
        s_Data2D.quadVA = VertexArray::Create();

        s_Data2D.triangleVB = VertexBuffer::Create(s_RendererData->MAX_VERTICES * sizeof(TriangleVertex));
        s_Data2D.quadVB = VertexBuffer::Create(s_RendererData->MAX_VERTICES * sizeof(QuadVertex));

        s_Data2D.triangleVB->SetLayout({
                { ShaderDataType::Float3, "a_Position" },
                { ShaderDataType::Float4, "a_Color" },
                { ShaderDataType::Float2, "a_TexCoord" },
                { ShaderDataType::Float, "a_TexIndex" },
                { ShaderDataType::Float, "a_TexScalar" }
            });
        s_Data2D.quadVB->SetLayout({
                { ShaderDataType::Float3, "a_Position" },
                { ShaderDataType::Float4, "a_Color" },
                { ShaderDataType::Float2, "a_TexCoord" },
                { ShaderDataType::Float, "a_TexIndex" },
                { ShaderDataType::Float, "a_TexScalar" }
            });

        s_Data2D.triangleVA->AddVertexBuffer(s_Data2D.triangleVB);
        s_Data2D.quadVA->AddVertexBuffer(s_Data2D.quadVB);

        s_Data2D.triangleVBBase = new TriangleVertex[s_RendererData->MAX_VERTICES];
        s_Data2D.quadVBBase = new QuadVertex[s_RendererData->MAX_VERTICES];

        uint32_t* triangleIndices = new uint32_t[s_RendererData->MAX_INDICES];
        for (uint32_t i = 0; i < s_RendererData->MAX_INDICES; i += 1) {
            triangleIndices[i] = i;
        }
        Ref<IndexBuffer> triangleIB = IndexBuffer::Create(triangleIndices, s_RendererData->MAX_INDICES);
        s_Data2D.triangleVA->AddIndexBuffer(triangleIB);
        delete[] triangleIndices;

        uint32_t* quadIndices = new uint32_t[s_RendererData->MAX_INDICES];
        uint32_t quadOffset = 0;
        for (uint32_t i = 0; i < s_RendererData->MAX_INDICES; i += 6)
        {
            quadIndices[i + 0] = quadOffset + 0;
            quadIndices[i + 1] = quadOffset + 1;
            quadIndices[i + 2] = quadOffset + 2;
            quadIndices[i + 3] = quadOffset + 2;
            quadIndices[i + 4] = quadOffset + 3;
            quadIndices[i + 5] = quadOffset + 0;
            quadOffset += 4;
        }
        Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, s_RendererData->MAX_INDICES);
        s_Data2D.quadVA->AddIndexBuffer(quadIB);
        delete[] quadIndices;

        s_Data2D.triangleShader = Shader::Create("Sandbox/assets/shaders/Triangle.glsl");
        s_Data2D.triangleShader->Bind();
        s_Data2D.triangleShader->SetIntArray("u_Textures", s_RendererData->textureSamplers, s_RendererData->MAX_TEXTURE_SLOTS);

        s_Data2D.quadShader = Shader::Create("Sandbox/assets/shaders/Quad.glsl");
        s_Data2D.quadShader->Bind();
        s_Data2D.quadShader->SetIntArray("u_Textures", s_RendererData->textureSamplers, s_RendererData->MAX_TEXTURE_SLOTS);

        s_Data2D.triangleVertPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
        s_Data2D.triangleVertPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
        s_Data2D.triangleVertPositions[2] = { -0.5f,  0.5f, 0.0f, 1.0f };

        s_Data2D.quadVertPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
        s_Data2D.quadVertPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
        s_Data2D.quadVertPositions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
        s_Data2D.quadVertPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

    }

    void Renderer2D::Shutdown()
    {
        DK_PROFILE_FUNC();

        delete[] s_Data2D.triangleVBBase;
        delete[] s_Data2D.quadVBBase;
    }

    void Renderer2D::BeginScene(const Camera& camera)
    {
        DK_PROFILE_FUNC();

        // Temp: remove once shader system can handle uniform bindings
        glm::mat4 viewProjection = camera.GetViewProjection();
        s_Data2D.quadShader->Bind();
        s_Data2D.quadShader->SetMat4("u_ViewProjection", viewProjection);
        s_Data2D.triangleShader->Bind();
        s_Data2D.triangleShader->SetMat4("u_ViewProjection", viewProjection);
    }

    void Renderer2D::BeginScene(const OrthographicCameraController& cameraController)
    {
        DK_PROFILE_FUNC();

        // Temp: ?? remove once shader system can handle uniform bindings
        glm::mat4 viewProjection = cameraController.GetCamera().GetViewProjection();
        // glm::vec3 viewPosition = cameraController.GetPosition(); // needed if doing lighting in 2D

        s_Data2D.quadShader->Bind();
        s_Data2D.quadShader->SetMat4("u_ViewProjection", viewProjection);
        s_Data2D.triangleShader->Bind();
        s_Data2D.triangleShader->SetMat4("u_ViewProjection", viewProjection);
    }

    void Renderer2D::BeginScene(const PerspectiveCameraController& cameraController)
    {
        DK_PROFILE_FUNC();

        // Temp: ?? remove once shader system can handle uniform bindings
        glm::mat4 viewProjection = cameraController.GetCamera().GetViewProjection();
        // glm::vec3 viewPosition = cameraController.GetPosition(); // needed if doing lighting in 2D

        s_Data2D.quadShader->Bind();
        s_Data2D.quadShader->SetMat4("u_ViewProjection", viewProjection);
        s_Data2D.triangleShader->Bind();
        s_Data2D.triangleShader->SetMat4("u_ViewProjection", viewProjection);
    }

    void Renderer2D::Flush()
    {
        DK_PROFILE_FUNC();

        if (s_Data2D.triangleIndexCount)
        {
            uint32_t bufferDataSize = (uint32_t)((uint8_t*)s_Data2D.triangleVBPtr - (uint8_t*)s_Data2D.triangleVBBase);
            s_Data2D.triangleVB->SetData(s_Data2D.triangleVBBase, bufferDataSize);

            for (uint32_t i = 0; i < s_RendererData->textureSlotIndex; i++)
                s_RendererData->textureSlots[i]->Bind(i);

            s_Data2D.triangleShader->Bind();
            RenderCommand::DrawIndexed(s_Data2D.triangleVA, s_Data2D.triangleIndexCount);
            s_RendererStats->drawCalls++;
        }

        if (s_Data2D.quadIndexCount)
        {
            uint32_t bufferDataSize = (uint32_t)((uint8_t*)s_Data2D.quadVBPtr - (uint8_t*)s_Data2D.quadVBBase);
            s_Data2D.quadVB->SetData(s_Data2D.quadVBBase, bufferDataSize);

            for (uint32_t i = 0; i < s_RendererData->textureSlotIndex; i++)
                s_RendererData->textureSlots[i]->Bind(i);

            s_Data2D.quadShader->Bind();
            RenderCommand::DrawIndexed(s_Data2D.quadVA, s_Data2D.quadIndexCount);
            s_RendererStats->drawCalls++;
        }

    }

    void Renderer2D::SetVBPointers()
    {
        s_Data2D.triangleVBPtr = s_Data2D.triangleVBBase;
        s_Data2D.quadVBPtr = s_Data2D.quadVBBase;
    }

    void Renderer2D::SetIndexCounts()
    {
        s_Data2D.triangleIndexCount = 0;
        s_Data2D.quadIndexCount = 0;
    }

    void Renderer2D::DrawTriangle(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        DK_PROFILE_FUNC();

        if (s_RendererData->totalIndices >= s_RendererData->MAX_INDICES)
            Renderer::NextBatch();

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), { position.x, position.y, 0.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        constexpr size_t verticesPerTriangle = 3;
        constexpr size_t indicesPerTriangle = 3;

        for (size_t i = 0; i < verticesPerTriangle; i++)
        {
            s_Data2D.triangleVBPtr->position = transform * s_Data2D.triangleVertPositions[i];
            s_Data2D.triangleVBPtr->color = color;
            s_Data2D.triangleVBPtr->textureCoord = glm::vec2((i % 2), (i / 2)); // Simple mapping
            s_Data2D.triangleVBPtr->textureIndex = 0.0f; // Default white texture
            s_Data2D.triangleVBPtr->textureScalar = 1.0f;
            s_Data2D.triangleVBPtr++;
        }

        s_Data2D.triangleIndexCount += indicesPerTriangle;
        s_RendererData->totalIndices += indicesPerTriangle;

        s_RendererStats->AddPrimitive(verticesPerTriangle, indicesPerTriangle);
    }

    void Renderer2D::DrawTriangle(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float textureScalar, const glm::vec4 textureTint)
    {
        DK_PROFILE_FUNC();

        if (s_RendererData->totalIndices >= s_RendererData->MAX_INDICES)
            Renderer::NextBatch();

        float textureIndex = 0.0f;

        for (uint32_t i = 1; i < s_RendererData->textureSlotIndex; i++)
        {
            if (*s_RendererData->textureSlots[i] == *texture)
            {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0)
        {
            if (s_RendererData->textureSlotIndex >= s_RendererData->MAX_TEXTURE_SLOTS)
                Renderer::NextBatch();

            textureIndex = (float)s_RendererData->textureSlotIndex;
            s_RendererData->textureSlots[s_RendererData->textureSlotIndex] = texture;
            s_RendererData->textureSlotIndex++;
        }

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), { position.x, position.y, 0.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        constexpr size_t verticesPerTriangle = 3;
        constexpr size_t indicesPerTriangle = 3;

        for (size_t i = 0; i < verticesPerTriangle; i++)
        {
            s_Data2D.triangleVBPtr->position = transform * s_Data2D.triangleVertPositions[i];
            s_Data2D.triangleVBPtr->color = textureTint;
            s_Data2D.triangleVBPtr->textureCoord = glm::vec2((i % 2), (i / 2)); // Simple mapping
            s_Data2D.triangleVBPtr->textureIndex = textureIndex;
            s_Data2D.triangleVBPtr->textureScalar = textureScalar;
            s_Data2D.triangleVBPtr++;
        }

        s_Data2D.triangleIndexCount += indicesPerTriangle;
        s_RendererData->totalIndices += indicesPerTriangle;

        s_RendererStats->AddPrimitive(verticesPerTriangle, indicesPerTriangle);
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        DK_PROFILE_FUNC();

        DrawQuad({ position.x, position.y, 0.0f }, size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
    {
        DK_PROFILE_FUNC();

        if (s_RendererData->totalIndices >= s_RendererData->MAX_INDICES)
            Renderer::NextBatch();

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), { position.x, position.y, position.z })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        constexpr size_t verticesPerQuad = 4;
        constexpr size_t indicesPerQuad = 6;
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

        for (size_t i = 0; i < verticesPerQuad; i++)
        {
            s_Data2D.quadVBPtr->position = transform * s_Data2D.quadVertPositions[i];
            s_Data2D.quadVBPtr->color = color;
            s_Data2D.quadVBPtr->textureCoord = textureCoords[i];
            s_Data2D.quadVBPtr->textureIndex = 0.0f; // Default white texture
            s_Data2D.quadVBPtr->textureScalar = 1.0f;
            s_Data2D.quadVBPtr++;
        }

        s_Data2D.quadIndexCount += indicesPerQuad;
        s_RendererData->totalIndices += indicesPerQuad;

        s_RendererStats->AddPrimitive(verticesPerQuad, indicesPerQuad);
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float textureScalar, const glm::vec4 textureTint)
    {
        DrawQuad({ position.x, position.y, 0.0f }, size, texture, textureScalar, textureTint);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float textureScalar, const glm::vec4 textureTint)
    {
        DK_PROFILE_FUNC();

        if (s_RendererData->totalIndices >= s_RendererData->MAX_INDICES)
            Renderer::NextBatch();

        float textureIndex = 0.0f;

        for (uint32_t i = 1; i < s_RendererData->textureSlotIndex; i++)
        {
            if (*s_RendererData->textureSlots[i] == *texture)
            {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0)
        {
            if (s_RendererData->textureSlotIndex >= s_RendererData->MAX_TEXTURE_SLOTS)
                Renderer::NextBatch();

            textureIndex = (float)s_RendererData->textureSlotIndex;
            s_RendererData->textureSlots[s_RendererData->textureSlotIndex] = texture;
            s_RendererData->textureSlotIndex++;
        }

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), { position.x, position.y, position.z })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        constexpr size_t verticesPerQuad = 4;
        constexpr size_t indicesPerQuad = 6;
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

        for (size_t i = 0; i < verticesPerQuad; i++)
        {
            s_Data2D.quadVBPtr->position = transform * s_Data2D.quadVertPositions[i];
            s_Data2D.quadVBPtr->color = textureTint;
            s_Data2D.quadVBPtr->textureCoord = textureCoords[i];
            s_Data2D.quadVBPtr->textureIndex = textureIndex;
            s_Data2D.quadVBPtr->textureScalar = textureScalar;
            s_Data2D.quadVBPtr++;
        }

        s_Data2D.quadIndexCount += indicesPerQuad;
        s_RendererData->totalIndices += indicesPerQuad;

        s_RendererStats->AddPrimitive(verticesPerQuad, indicesPerQuad);
    }

}

