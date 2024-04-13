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
        static const uint32_t maxTextureSlots = 16; //TODO RenderCaps

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

    static Renderer2DData s_2DData;

    void Renderer2D::Init()
    {
        DK_PROFILE_FUNC();

        s_2DData.vertexArray = VertexArray::Create();

        s_2DData.vertexBuffer = VertexBuffer::Create(s_2DData.maxVertices * sizeof(QuadVertex));
        s_2DData.vertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float4, "a_Color" },
            { ShaderDataType::Float2, "a_TexCoord" },
            { ShaderDataType::Float, "a_TexIndex" },
            { ShaderDataType::Float, "a_TexScalar" }
            });
        s_2DData.vertexArray->AddVertexBuffer(s_2DData.vertexBuffer);

        s_2DData.quadVertexBufferBase = new QuadVertex[s_2DData.maxVertices];

        uint32_t* indices = new uint32_t[s_2DData.maxIndices];

        uint32_t offset = 0;
        for (uint32_t i = 0; i < s_2DData.maxIndices; i += 6)
        {
            indices[i + 0] = offset + 0;
            indices[i + 1] = offset + 1;
            indices[i + 2] = offset + 2;

            indices[i + 3] = offset + 2;
            indices[i + 4] = offset + 3;
            indices[i + 5] = offset + 0;

            offset += 4;
        }

        Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indices, s_2DData.maxIndices);
        s_2DData.vertexArray->AddIndexBuffer(indexBuffer);
        delete[] indices;

        s_2DData.whiteTexture = Texture2D::Create(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        s_2DData.whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

        int32_t samplers[s_2DData.maxTextureSlots];
        for (uint32_t i = 0; i < s_2DData.maxTextureSlots; i++)
            samplers[i] = i;

        s_2DData.textureShader = Shader::Create("Sandbox/assets/shaders/Example2D.glsl");
        s_2DData.textureShader->Bind();
        s_2DData.textureShader->SetIntArray("u_Textures", samplers, s_2DData.maxTextureSlots);

        s_2DData.textureSlots[0] = s_2DData.whiteTexture;

        s_2DData.quadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
        s_2DData.quadVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
        s_2DData.quadVertexPositions[2] = { 0.5f, 0.5f, 0.0f, 1.0f };
        s_2DData.quadVertexPositions[3] = { -0.5f, 0.5f, 0.0f, 1.0f };
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

        s_2DData.quadIndexCount = 0;
        s_2DData.quadVertexBufferPtr = s_2DData.quadVertexBufferBase;

        s_2DData.textureSlotIndex = 1;
    }

    void Renderer2D::EndScene()
    {
        DK_PROFILE_FUNC();

        uint32_t dataSize = (uint32_t)((uint8_t*)s_2DData.quadVertexBufferPtr - (uint8_t*)s_2DData.quadVertexBufferBase);
        s_2DData.vertexBuffer->SetData(s_2DData.quadVertexBufferBase, dataSize);

        Flush();
    }

    void Renderer2D::Flush()
    {
        DK_PROFILE_FUNC();

        // Bind textures
        for (uint32_t i = 0; i < s_2DData.textureSlotIndex; i++)
            s_2DData.textureSlots[i]->Bind(i);

        RenderCommand::DrawIndexed(s_2DData.vertexArray, s_2DData.quadIndexCount);
        s_2DData.stats.drawCalls++;
    }

    void Renderer2D::FlushAndReset()
    {
        DK_PROFILE_FUNC();

        EndScene();

        s_2DData.quadIndexCount = 0;
        s_2DData.quadVertexBufferPtr = s_2DData.quadVertexBufferBase;

        s_2DData.textureSlotIndex = 1;
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        DrawQuad({ position.x, position.y, 0.0f }, size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
    {
        DK_PROFILE_FUNC();

        if (s_2DData.quadIndexCount >= Renderer2DData::maxIndices)
            FlushAndReset();

        glm::mat4 model = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        constexpr size_t vertexCount = 4;
        const float textureIndex = 0.0f; // White Texture
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
        const float textureScalar = 1.0f;

        for (size_t i = 0; i < vertexCount; i++)
        {
            s_2DData.quadVertexBufferPtr->position = model * s_2DData.quadVertexPositions[i];
            s_2DData.quadVertexBufferPtr->color = color;
            s_2DData.quadVertexBufferPtr->textureCoord = textureCoords[i];
            s_2DData.quadVertexBufferPtr->textureIndex = textureIndex;
            s_2DData.quadVertexBufferPtr->textureScalar = textureScalar;
            s_2DData.quadVertexBufferPtr++;
        }

        s_2DData.quadIndexCount += 6;

        s_2DData.stats.quadCount++;
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float textureScalar, const glm::vec4 textureTint)
    {
        DrawQuad({ position.x, position.y, 0.0f }, size, texture, textureScalar, textureTint);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float textureScalar, const glm::vec4 textureTint)
    {
        DK_PROFILE_FUNC();

        if (s_2DData.quadIndexCount >= Renderer2DData::maxIndices)
            FlushAndReset();

        float textureIndex = 0.0f;

        for (uint32_t i = 1; i < s_2DData.textureSlotIndex; i++)
        {
            if (*s_2DData.textureSlots[i] == *texture)
            {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0)
        {
            if (s_2DData.textureSlotIndex >= Renderer2DData::maxTextureSlots)
                FlushAndReset();

            textureIndex = (float)s_2DData.textureSlotIndex;
            s_2DData.textureSlots[s_2DData.textureSlotIndex] = texture;
            s_2DData.textureSlotIndex++;
        }

        glm::mat4 model = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        constexpr size_t vertexCount = 4;
        constexpr glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

        for (size_t i = 0; i < vertexCount; i++)
        {
            s_2DData.quadVertexBufferPtr->position = model * s_2DData.quadVertexPositions[i];
            s_2DData.quadVertexBufferPtr->color = textureTint;
            s_2DData.quadVertexBufferPtr->textureCoord = textureCoords[i];
            s_2DData.quadVertexBufferPtr->textureIndex = textureIndex;
            s_2DData.quadVertexBufferPtr->textureScalar = textureScalar;
            s_2DData.quadVertexBufferPtr++;
        }

        s_2DData.quadIndexCount += 6;

        s_2DData.stats.quadCount++;
    }

    void Renderer2D::DrawRotQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color)
    {
        DrawRotQuad({ position.x, position.y, 0.0f }, size, rotation, color);
    }

    void Renderer2D::DrawRotQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color)
    {
        DK_PROFILE_FUNC();

        if (s_2DData.quadIndexCount >= Renderer2DData::maxIndices)
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
            s_2DData.quadVertexBufferPtr->position = model * s_2DData.quadVertexPositions[i];
            s_2DData.quadVertexBufferPtr->color = color;
            s_2DData.quadVertexBufferPtr->textureCoord = textureCoords[i];
            s_2DData.quadVertexBufferPtr->textureIndex = textureIndex;
            s_2DData.quadVertexBufferPtr->textureScalar = textureScalar;
            s_2DData.quadVertexBufferPtr++;
        }

        s_2DData.quadIndexCount += 6;

        s_2DData.stats.quadCount++;
    }

    void Renderer2D::DrawRotQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float textureScalar, const glm::vec4 textureTint)
    {
        DrawRotQuad({ position.x, position.y, 0.0f }, size, rotation, texture, textureScalar, textureTint);
    }

    void Renderer2D::DrawRotQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float textureScalar, const glm::vec4 textureTint)
    {
        DK_PROFILE_FUNC();

        if (s_2DData.quadIndexCount >= Renderer2DData::maxIndices)
            FlushAndReset();

        float textureIndex = 0.0f;

        for (uint32_t i = 1; i < s_2DData.textureSlotIndex; i++)
        {
            if (*s_2DData.textureSlots[i] == *texture)
            {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0)
        {
            if (s_2DData.textureSlotIndex >= Renderer2DData::maxTextureSlots)
                FlushAndReset();

            textureIndex = (float)s_2DData.textureSlotIndex;
            s_2DData.textureSlots[s_2DData.textureSlotIndex] = texture;
            s_2DData.textureSlotIndex++;
        }

        glm::mat4 model = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        constexpr size_t vertexCount = 4;
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

        for (size_t i = 0; i < vertexCount; i++)
        {
            s_2DData.quadVertexBufferPtr->position = model * s_2DData.quadVertexPositions[i];
            s_2DData.quadVertexBufferPtr->color = textureTint;
            s_2DData.quadVertexBufferPtr->textureCoord = textureCoords[i];
            s_2DData.quadVertexBufferPtr->textureIndex = textureIndex;
            s_2DData.quadVertexBufferPtr->textureScalar = textureScalar;
            s_2DData.quadVertexBufferPtr++;
        }

        s_2DData.quadIndexCount += 6;

        s_2DData.stats.quadCount++;
    }

    void Renderer2D::ResetStats()
    {
        memset(&s_2DData.stats, 0, sizeof(Statistics));
    }

    Renderer2D::Statistics Renderer2D::GetStats()
    {
        return s_2DData.stats;
    }

}
