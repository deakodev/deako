#include "Renderer.h"
#include "dkpch.h"

#include "RenderCommand.h"  
#include "Renderer2D.h"
#include "Renderer3D.h"


namespace Deak {

    Ref<RendererStats> Renderer::s_Stats = CreateRef<RendererStats>();
    Ref<RendererData> Renderer::s_Data = CreateRef<RendererData>();

    void Renderer::Init()
    {
        DK_PROFILE_FUNC();

        SetDefaultTextureData();

        Renderer2D::Init();
        Renderer3D::Init();
        RenderCommand::Init();
    }

    void Renderer::Shutdown()
    {
        DK_PROFILE_FUNC();

        Renderer3D::Shutdown();
        Renderer2D::Shutdown();
    }

    void Renderer::BeginScene(const SceneCamera& camera, const glm::mat4& transform)
    {
        DK_PROFILE_FUNC();

        Renderer2D::PrepareScene(camera, transform);
        Renderer3D::PrepareScene(camera, transform);

        StartBatch();
    }

    void Renderer::BeginScene(const PerspectiveCameraController& cameraController, const glm::vec3& lightPosition)
    {
        DK_PROFILE_FUNC();

        Renderer2D::PrepareScene(cameraController);
        Renderer3D::PrepareScene(cameraController, lightPosition);

        StartBatch();
    }

    void Renderer::BeginScene(const OrthographicCameraController& cameraController)
    {
        DK_PROFILE_FUNC();

        Renderer2D::PrepareScene(cameraController);

        StartBatch();
    }

    void Renderer::EndScene()
    {
        DK_PROFILE_FUNC();

        Flush();
    }

    void Renderer::Flush()
    {
        if (s_Data->totalIndices == 0)
            return; // Nothing to draw

        DK_PROFILE_FUNC();

        Renderer2D::Flush();
        Renderer3D::Flush();
    }

    void Renderer::StartBatch()
    {
        DK_PROFILE_FUNC();

        Renderer2D::SetVBPointers();
        Renderer3D::SetVBPointers();

        Renderer2D::SetIndexCounts();
        Renderer3D::SetIndexCounts();

        s_Data->totalIndices = 0;
        s_Data->textureSlotIndex = 1;
    }

    void Renderer::NextBatch()
    {
        DK_PROFILE_FUNC();

        Flush();
        StartBatch();
    }

    void Renderer::ResetStats()
    {
        DK_PROFILE_FUNC();

        s_Stats->drawCalls = 0;
        s_Stats->primitiveCount = 0;
        s_Stats->vertexCount = 0;
        s_Stats->indexCount = 0;
    }

    void Renderer::SetDefaultTextureData()
    {
        DK_PROFILE_FUNC();

        s_Data->defaultTexture = Texture2D::Create(1, 1);
        uint32_t defaultTextureData = 0xffffffff;
        s_Data->defaultTexture->SetData(&defaultTextureData, sizeof(uint32_t));
        s_Data->textureSlots[0] = s_Data->defaultTexture;

        for (uint32_t i = 0; i < s_Data->MAX_TEXTURE_SLOTS; i++)
            s_Data->textureSamplers[i] = i;
    }

}
