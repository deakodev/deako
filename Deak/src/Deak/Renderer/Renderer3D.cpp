#include "Renderer3D.h"
#include "dkpch.h"

#include "Renderer.h"
#include "VertexArray.h"
#include "Buffer.h"
#include "Shader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Deak {

    struct CubeVertex
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec3 normal;
        glm::vec2 textureCoord;
        float textureIndex;
        float textureScalar;
    };

    struct PrimitiveData3D
    {
        // cube
        Ref<VertexArray> cubeVA;
        Ref<VertexBuffer> cubeVB;
        Ref<Shader> cubeShader;
        CubeVertex* cubeVBBase = nullptr;
        CubeVertex* cubeVBPtr = nullptr;;
        glm::vec4 cubeVertPositions[24];
        glm::vec3 cubeNormals[24];
        uint32_t cubeIndexCount;
    };

    static PrimitiveData3D s_Data3D;
    static Ref<RendererData> s_RendererData;
    static Ref<RendererStats> s_RendererStats;

    void Renderer3D::Init()
    {
        DK_PROFILE_FUNC();

        s_RendererData = Renderer::GetRendererData();
        s_RendererStats = Renderer::GetRendererStats();

        s_Data3D.cubeVA = VertexArray::Create();

        s_Data3D.cubeVB = VertexBuffer::Create(s_RendererData->MAX_VERTICES * sizeof(CubeVertex));

        s_Data3D.cubeVB->SetLayout({
                { ShaderDataType::Float3, "a_Position" },
                { ShaderDataType::Float4, "a_Color" },
                { ShaderDataType::Float3, "a_Normal" },
                { ShaderDataType::Float2, "a_TexCoord" },
                { ShaderDataType::Float, "a_TexIndex" },
                { ShaderDataType::Float, "a_TexScalar" }
            });

        s_Data3D.cubeVA->AddVertexBuffer(s_Data3D.cubeVB);

        s_Data3D.cubeVBBase = new CubeVertex[s_RendererData->MAX_VERTICES];

        uint32_t* cubeIndices = new uint32_t[s_RendererData->MAX_INDICES];
        uint32_t cubeOffset = 0;
        for (uint32_t i = 0; i < s_RendererData->MAX_INDICES; i += 6)
        {
            cubeIndices[i + 0] = cubeOffset + 0;
            cubeIndices[i + 1] = cubeOffset + 1;
            cubeIndices[i + 2] = cubeOffset + 2;
            cubeIndices[i + 3] = cubeOffset + 2;
            cubeIndices[i + 4] = cubeOffset + 3;
            cubeIndices[i + 5] = cubeOffset + 0;
            cubeOffset += 4;
        }
        Ref<IndexBuffer> cubeIB = IndexBuffer::Create(cubeIndices, s_RendererData->MAX_INDICES);
        s_Data3D.cubeVA->AddIndexBuffer(cubeIB);
        delete[] cubeIndices;

        s_Data3D.cubeShader = Shader::Create("Sandbox/assets/shaders/Cube.glsl");
        s_Data3D.cubeShader->Bind();
        s_Data3D.cubeShader->SetIntArray("u_Textures", s_RendererData->textureSamplers, s_RendererData->MAX_TEXTURE_SLOTS);

        s_Data3D.cubeVertPositions[0] = { -0.5f, -0.5f, 0.5f, 1.0f };   // Front bottom left
        s_Data3D.cubeVertPositions[1] = { 0.5f, -0.5f, 0.5f, 1.0f };    // Front bottom right
        s_Data3D.cubeVertPositions[2] = { 0.5f, 0.5f, 0.5f, 1.0f };     // Front top right
        s_Data3D.cubeVertPositions[3] = { -0.5f, 0.5f, 0.5f, 1.0f };    // Front top left

        s_Data3D.cubeVertPositions[4] = { -0.5f, -0.5f, -0.5f, 1.0f };  // Back bottom left
        s_Data3D.cubeVertPositions[5] = { 0.5f, -0.5f, -0.5f, 1.0f };   // Back bottom right
        s_Data3D.cubeVertPositions[6] = { 0.5f, 0.5f, -0.5f, 1.0f };    // Back top right
        s_Data3D.cubeVertPositions[7] = { -0.5f, 0.5f, -0.5f, 1.0f };   // Back top left

        s_Data3D.cubeVertPositions[8] = { -0.5f, 0.5f, 0.5f, 1.0f };    // Left top front
        s_Data3D.cubeVertPositions[9] = { -0.5f, 0.5f, -0.5f, 1.0f };   // Left top back
        s_Data3D.cubeVertPositions[10] = { -0.5f, -0.5f, -0.5f, 1.0f }; // Left bottom back
        s_Data3D.cubeVertPositions[11] = { -0.5f, -0.5f, 0.5f, 1.0f };  // Left bottom front

        s_Data3D.cubeVertPositions[12] = { 0.5f, 0.5f, 0.5f, 1.0f };    // Right top front
        s_Data3D.cubeVertPositions[13] = { 0.5f, 0.5f, -0.5f, 1.0f };   // Right top back
        s_Data3D.cubeVertPositions[14] = { 0.5f, -0.5f, -0.5f, 1.0f };  // Right bottom back
        s_Data3D.cubeVertPositions[15] = { 0.5f, -0.5f, 0.5f, 1.0f };   // Right bottom front

        s_Data3D.cubeVertPositions[16] = { -0.5f, 0.5f, -0.5f, 1.0f };  // Top back left
        s_Data3D.cubeVertPositions[17] = { -0.5f, 0.5f, 0.5f, 1.0f };   // Top front left
        s_Data3D.cubeVertPositions[18] = { 0.5f, 0.5f, 0.5f, 1.0f };    // Top front right
        s_Data3D.cubeVertPositions[19] = { 0.5f, 0.5f, -0.5f, 1.0f };   // Top back right

        s_Data3D.cubeVertPositions[20] = { -0.5f, -0.5f, -0.5f, 1.0f }; // Bottom back left
        s_Data3D.cubeVertPositions[21] = { -0.5f, -0.5f, 0.5f, 1.0f };  // Bottom front left
        s_Data3D.cubeVertPositions[22] = { 0.5f, -0.5f, 0.5f, 1.0f };   // Bottom front right
        s_Data3D.cubeVertPositions[23] = { 0.5f, -0.5f, -0.5f, 1.0f };  // Bottom back right

        s_Data3D.cubeNormals[0] = { 0.0f, 0.0f, 1.0f };   // Front bottom left
        s_Data3D.cubeNormals[1] = { 0.0f, 0.0f, 1.0f };    // Front bottom right
        s_Data3D.cubeNormals[2] = { 0.0f, 0.0f, 1.0f };     // Front top right
        s_Data3D.cubeNormals[3] = { 0.0f, 0.0f, 1.0f };    // Front top left

        s_Data3D.cubeNormals[4] = { 0.0f, 0.0f, -1.0f };  // Back bottom left
        s_Data3D.cubeNormals[5] = { 0.0f, 0.0f, -1.0f };   // Back bottom right
        s_Data3D.cubeNormals[6] = { 0.0f, 0.0f, -1.0f };    // Back top right
        s_Data3D.cubeNormals[7] = { 0.0f, 0.0f, -1.0f };   // Back top left

        s_Data3D.cubeNormals[8] = { -1.0f, 0.0f, 0.0f };    // Left top front
        s_Data3D.cubeNormals[9] = { -1.0f, 0.0f, 0.0f };   // Left top back
        s_Data3D.cubeNormals[10] = { -1.0f, 0.0f, 0.0f }; // Left bottom back
        s_Data3D.cubeNormals[11] = { -1.0f, 0.0f, 0.0f };  // Left bottom front

        s_Data3D.cubeNormals[12] = { 1.0f, 0.0f, 0.0f };    // Right top front
        s_Data3D.cubeNormals[13] = { 1.0f, 0.0f, 0.0f };   // Right top back
        s_Data3D.cubeNormals[14] = { 1.0f, 0.0f, 0.0f };  // Right bottom back
        s_Data3D.cubeNormals[15] = { 1.0f, 0.0f, 0.0f };   // Right bottom front

        s_Data3D.cubeNormals[16] = { 0.0f, 1.0f, 0.0f };  // Top back left
        s_Data3D.cubeNormals[17] = { 0.0f, 1.0f, 0.0f };   // Top front left
        s_Data3D.cubeNormals[18] = { 0.0f, 1.0f, 0.0f };    // Top front right
        s_Data3D.cubeNormals[19] = { 0.0f, 1.0f, 0.0f };   // Top back right

        s_Data3D.cubeNormals[20] = { 0.0f, -1.0f, 0.0f }; // Bottom back left
        s_Data3D.cubeNormals[21] = { 0.0f, -1.0f, 0.0f };  // Bottom front left
        s_Data3D.cubeNormals[22] = { 0.0f, -1.0f, 0.0f };   // Bottom front right
        s_Data3D.cubeNormals[23] = { 0.0f, -1.0f, 0.0f };  // Bottom back right

    }

    void Renderer3D::Shutdown()
    {
        DK_PROFILE_FUNC();

        delete[] s_Data3D.cubeVBBase;
    }

    void Renderer3D::BeginScene(const PerspectiveCameraController& cameraController, const glm::vec3& lightPosition)
    {
        DK_PROFILE_FUNC();

        // Temp: ?? remove once shader system can handle uniform bindings
        glm::mat4 viewProjection = cameraController.GetCamera().GetViewProjection();
        glm::vec3 viewPosition = cameraController.GetPosition();

        s_Data3D.cubeShader->Bind();
        s_Data3D.cubeShader->SetMat4("u_ViewProjection", viewProjection);
        s_Data3D.cubeShader->SetFloat3("u_ViewPosition", viewPosition);
        s_Data3D.cubeShader->SetFloat3("u_LightPosition", lightPosition);
    }

    void Renderer3D::Flush()
    {
        DK_PROFILE_FUNC();

        if (s_Data3D.cubeIndexCount)
        {
            uint32_t bufferDataSize = (uint32_t)((uint8_t*)s_Data3D.cubeVBPtr - (uint8_t*)s_Data3D.cubeVBBase);
            s_Data3D.cubeVB->SetData(s_Data3D.cubeVBBase, bufferDataSize);

            for (uint32_t i = 0; i < s_RendererData->textureSlotIndex; i++)
                s_RendererData->textureSlots[i]->Bind(i);

            s_Data3D.cubeShader->Bind();
            RenderCommand::DrawIndexed(s_Data3D.cubeVA, s_Data3D.cubeIndexCount);
            s_RendererStats->drawCalls++;
        }

    }

    void Renderer3D::SetVBPointers()
    {
        s_Data3D.cubeVBPtr = s_Data3D.cubeVBBase;
    }

    void Renderer3D::SetIndexCounts()
    {
        s_Data3D.cubeIndexCount = 0;
    }

    void Renderer3D::DrawCube(const glm::vec3& position, const glm::vec3& size, const glm::vec4& color)
    {
        DK_PROFILE_FUNC();

        if (s_RendererData->totalIndices >= s_RendererData->MAX_INDICES)
            Renderer::NextBatch();

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), { position.x, position.y, position.z })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, size.z });

        constexpr size_t verticesPerCube = 24;
        constexpr size_t indicesPerCube = 36;
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

        for (size_t i = 0; i < verticesPerCube; i++)
        {
            s_Data3D.cubeVBPtr->position = transform * s_Data3D.cubeVertPositions[i];
            s_Data3D.cubeVBPtr->color = color;
            s_Data3D.cubeVBPtr->normal = s_Data3D.cubeNormals[i];
            s_Data3D.cubeVBPtr->textureCoord = textureCoords[i % 4];
            s_Data3D.cubeVBPtr->textureIndex = 0.0f; // Default white texture
            s_Data3D.cubeVBPtr->textureScalar = 1.0f;
            s_Data3D.cubeVBPtr++;
        }

        s_Data3D.cubeIndexCount += indicesPerCube;
        s_RendererData->totalIndices += indicesPerCube;

        s_RendererStats->AddPrimitive(verticesPerCube, indicesPerCube, 6);
    }

    void Renderer3D::DrawCube(const glm::vec3& position, const glm::vec3& size, const Ref<Texture2D>& texture, float textureScalar, const glm::vec4 textureTint)
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

        constexpr size_t verticesPerCube = 24;
        constexpr size_t indicesPerCube = 36;
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

        for (size_t i = 0; i < verticesPerCube; i++)
        {
            s_Data3D.cubeVBPtr->position = transform * s_Data3D.cubeVertPositions[i];
            s_Data3D.cubeVBPtr->color = textureTint;
            s_Data3D.cubeVBPtr->normal = s_Data3D.cubeNormals[i];
            s_Data3D.cubeVBPtr->textureCoord = textureCoords[i % 4];
            s_Data3D.cubeVBPtr->textureIndex = textureIndex;
            s_Data3D.cubeVBPtr->textureScalar = textureScalar;
            s_Data3D.cubeVBPtr++;
        }

        s_Data3D.cubeIndexCount += indicesPerCube;
        s_RendererData->totalIndices += indicesPerCube;

        s_RendererStats->AddPrimitive(verticesPerCube, indicesPerCube, 6);
    }

}

