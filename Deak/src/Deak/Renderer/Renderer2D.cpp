#include "Renderer2D.h"
#include "dkpch.h"

#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Deak {

    struct QuadVertex
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 textureCoord;
        float textureIndex;
        float textureScalar;
    };

    struct Renderer2DData
    {
        static const uint32_t maxQuads = 20000;
        static const uint32_t maxVertices = maxQuads * 4;
        static const uint32_t maxIndices = maxQuads * 6;
        static const uint32_t maxTextureSlots = 16;

        Ref<VertexArray> vertexArray;
        Ref<VertexBuffer> vertexBuffer;
        Ref<Shader> textureShader;
        Ref<Texture2D> whiteTexture;

        uint32_t quadIndexCount = 0;
        QuadVertex* quadVertexBufferBase = nullptr;
        QuadVertex* quadVertexBufferPtr = nullptr;

        std::array<Ref<Texture2D>, maxTextureSlots> textureSlots;
        uint32_t textureSlotIndex = 1; // 0 is our white texture

        glm::vec4 quadVertexPositions[4];

        Renderer2D::Statistics stats;
    };

    static Renderer2DData s_Data2D;

    void Renderer2D::Init()
    {
        DK_PROFILE_FUNC();

        s_Data2D.vertexArray = VertexArray::Create();

        s_Data2D.vertexBuffer = VertexBuffer::Create(s_Data2D.maxVertices * sizeof(QuadVertex));
        s_Data2D.vertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float4, "a_Color" },
            { ShaderDataType::Float2, "a_TexCoord" },
            { ShaderDataType::Float, "a_TexIndex" },
            { ShaderDataType::Float, "a_TexScalar" }
            });
        s_Data2D.vertexArray->AddVertexBuffer(s_Data2D.vertexBuffer);

        s_Data2D.quadVertexBufferBase = new QuadVertex[s_Data2D.maxVertices];

        uint32_t* indices = new uint32_t[s_Data2D.maxIndices];

        uint32_t offset = 0;
        for (uint32_t i = 0; i < s_Data2D.maxIndices; i += 6)
        {
            indices[i + 0] = offset + 0;
            indices[i + 1] = offset + 1;
            indices[i + 2] = offset + 2;

            indices[i + 3] = offset + 2;
            indices[i + 4] = offset + 3;
            indices[i + 5] = offset + 0;

            offset += 4;
        }

        Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indices, s_Data2D.maxIndices);
        s_Data2D.vertexArray->AddIndexBuffer(indexBuffer);
        delete[] indices;

        s_Data2D.whiteTexture = Texture2D::Create(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        s_Data2D.whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

        int32_t samplers[s_Data2D.maxTextureSlots];
        for (uint32_t i = 0; i < s_Data2D.maxTextureSlots; i++)
            samplers[i] = i;

        s_Data2D.textureShader = Shader::Create("Sandbox/assets/shaders/Example2D.glsl");
        s_Data2D.textureShader->Bind();
        s_Data2D.textureShader->SetIntArray("u_Textures", samplers, s_Data2D.maxTextureSlots);

        s_Data2D.textureSlots[0] = s_Data2D.whiteTexture;

        s_Data2D.quadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
        s_Data2D.quadVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
        s_Data2D.quadVertexPositions[2] = { 0.5f, 0.5f, 0.0f, 1.0f };
        s_Data2D.quadVertexPositions[3] = { -0.5f, 0.5f, 0.0f, 1.0f };
    }

    void Renderer2D::Shutdown()
    {
        DK_PROFILE_FUNC();
    }

    void Renderer2D::BeginScene(const OrthographicCamera& camera)
    {
        DK_PROFILE_FUNC();

        s_Data2D.textureShader->Bind();
        s_Data2D.textureShader->SetMat4("u_ViewProjection", camera.GetViewProjection());

        s_Data2D.quadIndexCount = 0;
        s_Data2D.quadVertexBufferPtr = s_Data2D.quadVertexBufferBase;

        s_Data2D.textureSlotIndex = 1;
    }

    void Renderer2D::EndScene()
    {
        DK_PROFILE_FUNC();

        uint32_t dataSize = (uint32_t)((uint8_t*)s_Data2D.quadVertexBufferPtr - (uint8_t*)s_Data2D.quadVertexBufferBase);
        s_Data2D.vertexBuffer->SetData(s_Data2D.quadVertexBufferBase, dataSize);

        Flush();
    }

    void Renderer2D::Flush()
    {
        DK_PROFILE_FUNC();

        // Bind textures
        for (uint32_t i = 0; i < s_Data2D.textureSlotIndex; i++)
            s_Data2D.textureSlots[i]->Bind(i);

        RenderCommand::DrawIndexed(s_Data2D.vertexArray, s_Data2D.quadIndexCount);
        s_Data2D.stats.drawCalls++;
    }

    void Renderer2D::FlushAndReset()
    {
        DK_PROFILE_FUNC();

        EndScene();

        s_Data2D.quadIndexCount = 0;
        s_Data2D.quadVertexBufferPtr = s_Data2D.quadVertexBufferBase;

        s_Data2D.textureSlotIndex = 1;
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        DrawQuad({ position.x, position.y, 0.0f }, size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
    {
        DK_PROFILE_FUNC();

        if (s_Data2D.quadIndexCount >= Renderer2DData::maxIndices)
            FlushAndReset();

        glm::mat4 model = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        constexpr size_t vertexCount = 4;
        const float textureIndex = 0.0f; // White Texture
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
        const float textureScalar = 1.0f;

        for (size_t i = 0; i < vertexCount; i++)
        {
            s_Data2D.quadVertexBufferPtr->position = model * s_Data2D.quadVertexPositions[i];
            s_Data2D.quadVertexBufferPtr->color = color;
            s_Data2D.quadVertexBufferPtr->textureCoord = textureCoords[i];
            s_Data2D.quadVertexBufferPtr->textureIndex = textureIndex;
            s_Data2D.quadVertexBufferPtr->textureScalar = textureScalar;
            s_Data2D.quadVertexBufferPtr++;
        }

        s_Data2D.quadIndexCount += 6;

        s_Data2D.stats.quadCount++;
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float textureScalar, const glm::vec4 textureTint)
    {
        DrawQuad({ position.x, position.y, 0.0f }, size, texture, textureScalar, textureTint);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float textureScalar, const glm::vec4 textureTint)
    {
        DK_PROFILE_FUNC();

        if (s_Data2D.quadIndexCount >= Renderer2DData::maxIndices)
            FlushAndReset();

        float textureIndex = 0.0f;

        for (uint32_t i = 1; i < s_Data2D.textureSlotIndex; i++)
        {
            if (*s_Data2D.textureSlots[i] == *texture)
            {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0)
        {
            if (s_Data2D.textureSlotIndex >= Renderer2DData::maxTextureSlots)
                FlushAndReset();

            textureIndex = (float)s_Data2D.textureSlotIndex;
            s_Data2D.textureSlots[s_Data2D.textureSlotIndex] = texture;
            s_Data2D.textureSlotIndex++;
        }

        glm::mat4 model = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        constexpr size_t vertexCount = 4;
        constexpr glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

        for (size_t i = 0; i < vertexCount; i++)
        {
            s_Data2D.quadVertexBufferPtr->position = model * s_Data2D.quadVertexPositions[i];
            s_Data2D.quadVertexBufferPtr->color = textureTint;
            s_Data2D.quadVertexBufferPtr->textureCoord = textureCoords[i];
            s_Data2D.quadVertexBufferPtr->textureIndex = textureIndex;
            s_Data2D.quadVertexBufferPtr->textureScalar = textureScalar;
            s_Data2D.quadVertexBufferPtr++;
        }

        s_Data2D.quadIndexCount += 6;

        s_Data2D.stats.quadCount++;
    }

    void Renderer2D::DrawRotQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color)
    {
        DrawRotQuad({ position.x, position.y, 0.0f }, size, rotation, color);
    }

    void Renderer2D::DrawRotQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color)
    {
        DK_PROFILE_FUNC();

        if (s_Data2D.quadIndexCount >= Renderer2DData::maxIndices)
            FlushAndReset();

        glm::mat4 model = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        constexpr size_t vertexCount = 4;
        const float textureIndex = 0.0f; // White Texture
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
        const float textureScalar = 1.0f;

        for (size_t i = 0; i < vertexCount; i++)
        {
            s_Data2D.quadVertexBufferPtr->position = model * s_Data2D.quadVertexPositions[i];
            s_Data2D.quadVertexBufferPtr->color = color;
            s_Data2D.quadVertexBufferPtr->textureCoord = textureCoords[i];
            s_Data2D.quadVertexBufferPtr->textureIndex = textureIndex;
            s_Data2D.quadVertexBufferPtr->textureScalar = textureScalar;
            s_Data2D.quadVertexBufferPtr++;
        }

        s_Data2D.quadIndexCount += 6;

        s_Data2D.stats.quadCount++;
    }

    void Renderer2D::DrawRotQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float textureScalar, const glm::vec4 textureTint)
    {
        DrawRotQuad({ position.x, position.y, 0.0f }, size, rotation, texture, textureScalar, textureTint);
    }

    void Renderer2D::DrawRotQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float textureScalar, const glm::vec4 textureTint)
    {
        DK_PROFILE_FUNC();

        if (s_Data2D.quadIndexCount >= Renderer2DData::maxIndices)
            FlushAndReset();

        float textureIndex = 0.0f;

        for (uint32_t i = 1; i < s_Data2D.textureSlotIndex; i++)
        {
            if (*s_Data2D.textureSlots[i] == *texture)
            {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0)
        {
            if (s_Data2D.textureSlotIndex >= Renderer2DData::maxTextureSlots)
                FlushAndReset();

            textureIndex = (float)s_Data2D.textureSlotIndex;
            s_Data2D.textureSlots[s_Data2D.textureSlotIndex] = texture;
            s_Data2D.textureSlotIndex++;
        }

        glm::mat4 model = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        constexpr size_t vertexCount = 4;
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

        for (size_t i = 0; i < vertexCount; i++)
        {
            s_Data2D.quadVertexBufferPtr->position = model * s_Data2D.quadVertexPositions[i];
            s_Data2D.quadVertexBufferPtr->color = textureTint;
            s_Data2D.quadVertexBufferPtr->textureCoord = textureCoords[i];
            s_Data2D.quadVertexBufferPtr->textureIndex = textureIndex;
            s_Data2D.quadVertexBufferPtr->textureScalar = textureScalar;
            s_Data2D.quadVertexBufferPtr++;
        }

        s_Data2D.quadIndexCount += 6;

        s_Data2D.stats.quadCount++;
    }

    void Renderer2D::ResetStats()
    {
        memset(&s_Data2D.stats, 0, sizeof(Statistics));
    }

    Renderer2D::Statistics Renderer2D::GetStats()
    {
        return s_Data2D.stats;
    }

}
