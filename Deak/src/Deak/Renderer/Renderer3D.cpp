#include "Renderer3D.h"
#include "dkpch.h"

#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Deak {

    struct CubeVertex
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 textureCoord;
        float textureIndex;
        float textureScalar;
    };

    struct Renderer3DData
    {
        static const uint32_t maxCubes = 10000;
        static const uint32_t maxVertices = maxCubes * 24;
        static const uint32_t maxIndices = maxCubes * 36;
        static const uint32_t maxTextureSlots = 16;

        Ref<VertexArray> vertexArray;
        Ref<VertexBuffer> vertexBuffer;
        Ref<Shader> textureShader;
        Ref<Texture2D> whiteTexture;

        uint32_t cubeIndexCount = 0;
        CubeVertex* cubeVertexBufferBase = nullptr;
        CubeVertex* cubeVertexBufferPtr = nullptr;

        std::array<Ref<Texture2D>, maxTextureSlots> textureSlots;
        uint32_t textureSlotIndex = 1; // 0 is our white texture

        glm::vec4 cubeVertexPositions[24];

        Renderer3D::Statistics stats;
    };

    static Renderer3DData s_Data3D;

    void Renderer3D::Init()
    {
        DK_PROFILE_FUNC();

        s_Data3D.vertexArray = VertexArray::Create();

        s_Data3D.vertexBuffer = VertexBuffer::Create(s_Data3D.maxVertices * sizeof(CubeVertex));
        s_Data3D.vertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float4, "a_Color" },
            { ShaderDataType::Float2, "a_TexCoord" },
            { ShaderDataType::Float, "a_TexIndex" },
            { ShaderDataType::Float, "a_TexScalar" }
            });
        s_Data3D.vertexArray->AddVertexBuffer(s_Data3D.vertexBuffer);

        s_Data3D.cubeVertexBufferBase = new CubeVertex[s_Data3D.maxVertices];

        uint32_t* indices = new uint32_t[s_Data3D.maxIndices];
        uint32_t index = 0; // This will track the current position in the indices array

        for (uint32_t cube = 0; cube < s_Data3D.maxCubes; cube++)
        {
            uint32_t offset = cube * 24; // Start of the current cube's vertex block

            for (uint32_t face = 0; face < 6; face++)
            {
                // Calculate the starting vertex for this face
                uint32_t faceOffset = offset + face * 4;

                // Define two triangles for the current face
                indices[index++] = faceOffset + 0;
                indices[index++] = faceOffset + 1;
                indices[index++] = faceOffset + 2;
                indices[index++] = faceOffset + 2;
                indices[index++] = faceOffset + 3;
                indices[index++] = faceOffset + 0;
            }
        }

        Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indices, s_Data3D.maxIndices);
        s_Data3D.vertexArray->AddIndexBuffer(indexBuffer);
        delete[] indices;

        s_Data3D.whiteTexture = Texture2D::Create(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        s_Data3D.whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

        int32_t samplers[s_Data3D.maxTextureSlots];
        for (uint32_t i = 0; i < s_Data3D.maxTextureSlots; i++)
            samplers[i] = i;

        s_Data3D.textureShader = Shader::Create("Sandbox/assets/shaders/Example3D.glsl");
        s_Data3D.textureShader->Bind();
        s_Data3D.textureShader->SetIntArray("u_Textures", samplers, s_Data3D.maxTextureSlots);

        s_Data3D.textureSlots[0] = s_Data3D.whiteTexture;

        // Positions for each vertex of a cube
        s_Data3D.cubeVertexPositions[0] = { -0.5f, -0.5f,  0.5f, 1.0f };  // Front bottom left
        s_Data3D.cubeVertexPositions[1] = { 0.5f, -0.5f,  0.5f, 1.0f };  // Front bottom right
        s_Data3D.cubeVertexPositions[2] = { 0.5f,  0.5f,  0.5f, 1.0f };  // Front top right
        s_Data3D.cubeVertexPositions[3] = { -0.5f,  0.5f,  0.5f, 1.0f };  // Front top left

        s_Data3D.cubeVertexPositions[4] = { -0.5f, -0.5f, -0.5f, 1.0f };  // Back bottom left
        s_Data3D.cubeVertexPositions[5] = { 0.5f, -0.5f, -0.5f, 1.0f };  // Back bottom right
        s_Data3D.cubeVertexPositions[6] = { 0.5f,  0.5f, -0.5f, 1.0f };  // Back top right
        s_Data3D.cubeVertexPositions[7] = { -0.5f,  0.5f, -0.5f, 1.0f };  // Back top left

        s_Data3D.cubeVertexPositions[8] = { -0.5f,  0.5f,  0.5f, 1.0f };  // Left top front
        s_Data3D.cubeVertexPositions[9] = { -0.5f,  0.5f, -0.5f, 1.0f };  // Left top back
        s_Data3D.cubeVertexPositions[10] = { -0.5f, -0.5f, -0.5f, 1.0f }; // Left bottom back
        s_Data3D.cubeVertexPositions[11] = { -0.5f, -0.5f,  0.5f, 1.0f }; // Left bottom front

        s_Data3D.cubeVertexPositions[12] = { 0.5f,  0.5f,  0.5f, 1.0f };  // Right top front
        s_Data3D.cubeVertexPositions[13] = { 0.5f,  0.5f, -0.5f, 1.0f };  // Right top back
        s_Data3D.cubeVertexPositions[14] = { 0.5f, -0.5f, -0.5f, 1.0f };  // Right bottom back
        s_Data3D.cubeVertexPositions[15] = { 0.5f, -0.5f,  0.5f, 1.0f };  // Right bottom front

        s_Data3D.cubeVertexPositions[16] = { -0.5f,  0.5f, -0.5f, 1.0f }; // Top back left
        s_Data3D.cubeVertexPositions[17] = { -0.5f,  0.5f,  0.5f, 1.0f }; // Top front left
        s_Data3D.cubeVertexPositions[18] = { 0.5f,  0.5f,  0.5f, 1.0f }; // Top front right
        s_Data3D.cubeVertexPositions[19] = { 0.5f,  0.5f, -0.5f, 1.0f }; // Top back right

        s_Data3D.cubeVertexPositions[20] = { -0.5f, -0.5f, -0.5f, 1.0f }; // Bottom back left
        s_Data3D.cubeVertexPositions[21] = { -0.5f, -0.5f,  0.5f, 1.0f }; // Bottom front left
        s_Data3D.cubeVertexPositions[22] = { 0.5f, -0.5f,  0.5f, 1.0f }; // Bottom front right
        s_Data3D.cubeVertexPositions[23] = { 0.5f, -0.5f, -0.5f, 1.0f }; // Bottom back right

    }

    void Renderer3D::Shutdown()
    {
        DK_PROFILE_FUNC();
    }

    void Renderer3D::BeginScene(const PerspectiveCamera& camera)
    {
        DK_PROFILE_FUNC();

        s_Data3D.textureShader->Bind();
        s_Data3D.textureShader->SetMat4("u_ViewProjection", camera.GetViewProjection());

        s_Data3D.cubeIndexCount = 0;
        s_Data3D.cubeVertexBufferPtr = s_Data3D.cubeVertexBufferBase;

        s_Data3D.textureSlotIndex = 1;
    }

    void Renderer3D::EndScene()
    {
        DK_PROFILE_FUNC();

        uint32_t dataSize = (uint32_t)((uint8_t*)s_Data3D.cubeVertexBufferPtr - (uint8_t*)s_Data3D.cubeVertexBufferBase);
        s_Data3D.vertexBuffer->SetData(s_Data3D.cubeVertexBufferBase, dataSize);

        Flush();
    }

    void Renderer3D::Flush()
    {
        DK_PROFILE_FUNC();

        // Bind textures
        for (uint32_t i = 0; i < s_Data3D.textureSlotIndex; i++)
            s_Data3D.textureSlots[i]->Bind(i);

        RenderCommand::DrawIndexed(s_Data3D.vertexArray, s_Data3D.cubeIndexCount);
        s_Data3D.stats.drawCalls++;
    }

    void Renderer3D::FlushAndReset()
    {
        DK_PROFILE_FUNC();

        EndScene();

        s_Data3D.cubeIndexCount = 0;
        s_Data3D.cubeVertexBufferPtr = s_Data3D.cubeVertexBufferBase;

        s_Data3D.textureSlotIndex = 1;
    }

    void Renderer3D::DrawCube(const glm::vec3& position, const glm::vec3& size, const glm::vec4& color)
    {
        DK_PROFILE_FUNC();

        if (s_Data3D.cubeIndexCount >= Renderer3DData::maxIndices)
            FlushAndReset();

        glm::mat4 model = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), size);

        constexpr size_t vertexCount = 24;
        const float textureIndex = 0.0f; // White Texture
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
        const float textureScalar = 1.0f;

        for (size_t i = 0; i < vertexCount; i++)
        {
            s_Data3D.cubeVertexBufferPtr->position = model * s_Data3D.cubeVertexPositions[i];
            s_Data3D.cubeVertexBufferPtr->color = color;
            s_Data3D.cubeVertexBufferPtr->textureCoord = textureCoords[i % 4];
            s_Data3D.cubeVertexBufferPtr->textureIndex = textureIndex;
            s_Data3D.cubeVertexBufferPtr->textureScalar = textureScalar;
            s_Data3D.cubeVertexBufferPtr++;
        }

        s_Data3D.cubeIndexCount += 36;

        s_Data3D.stats.cubeCount++;
    }

    void Renderer3D::DrawCube(const glm::vec3& position, const glm::vec3& size, const Ref<Texture2D>& texture, float textureScalar, const glm::vec4 textureTint)
    {
        DK_PROFILE_FUNC();

        if (s_Data3D.cubeIndexCount >= Renderer3DData::maxIndices)
            FlushAndReset();

        float textureIndex = 0.0f;

        for (uint32_t i = 1; i < s_Data3D.textureSlotIndex; i++)
        {
            if (*s_Data3D.textureSlots[i] == *texture)
            {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0)
        {
            if (s_Data3D.textureSlotIndex >= Renderer3DData::maxTextureSlots)
                FlushAndReset();

            textureIndex = (float)s_Data3D.textureSlotIndex;
            s_Data3D.textureSlots[s_Data3D.textureSlotIndex] = texture;
            s_Data3D.textureSlotIndex++;
        }

        glm::mat4 model = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), size);

        constexpr size_t vertexCount = 24;
        constexpr glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

        for (size_t i = 0; i < vertexCount; i++)
        {
            s_Data3D.cubeVertexBufferPtr->position = model * s_Data3D.cubeVertexPositions[i];
            s_Data3D.cubeVertexBufferPtr->color = textureTint;
            s_Data3D.cubeVertexBufferPtr->textureCoord = textureCoords[i % 4];
            s_Data3D.cubeVertexBufferPtr->textureIndex = textureIndex;
            s_Data3D.cubeVertexBufferPtr->textureScalar = textureScalar;
            s_Data3D.cubeVertexBufferPtr++;
        }

        s_Data3D.cubeIndexCount += 36;

        s_Data3D.stats.cubeCount++;
    }

    void Renderer3D::ResetStats()
    {
        memset(&s_Data3D.stats, 0, sizeof(Statistics));
    }

    Renderer3D::Statistics Renderer3D::GetStats()
    {
        return s_Data3D.stats;
    }


    // void Renderer3D::DrawQuad(const glm::vec2& position, const glm::vec3& size, const glm::vec4& color)
    // {
    //     DrawQuad({ position.x, position.y, 0.0f }, size, color);
    // }

    // void Renderer3D::DrawQuad(const glm::vec3& position, const glm::vec3& size, const glm::vec4& color)
    // {
    //     DK_PROFILE_FUNC();

    //     s_Data3D.textureShader->SetFloat4("u_Color", color);
    //     s_Data3D.whiteTexture->Bind();

    //     glm::mat4 model = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 1.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), { size.x, size.y, size.z });
    //     s_Data3D.textureShader->SetMat4("u_Model", model);

    //     s_Data3D.vertexArray->Bind();
    //     RenderCommand::DrawIndexed(s_Data3D.vertexArray);
    // }

    // void Renderer3D::DrawQuad(const glm::vec2& position, const glm::vec3& size, const Ref<Texture2D>& texture)
    // {
    //     DrawQuad({ position.x, position.y, 0.0f }, size, texture);
    // }

    // void Renderer3D::DrawQuad(const glm::vec3& position, const glm::vec3& size, const Ref<Texture2D>& texture)
    // {
    //     DK_PROFILE_FUNC();

    //     s_Data3D.textureShader->SetFloat4("u_Color", glm::vec4(1.0f));
    //     texture->Bind();

    //     glm::mat4 model = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 1.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), { size.x, size.y, size.z });
    //     s_Data3D.textureShader->SetMat4("u_Model", model);

    //     s_Data3D.vertexArray->Bind();
    //     RenderCommand::DrawIndexed(s_Data3D.vertexArray);
    // }

}
