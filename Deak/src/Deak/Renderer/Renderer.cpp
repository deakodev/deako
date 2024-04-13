#include "Renderer.h"
#include "dkpch.h"

#include "Renderer2D.h"
#include "Renderer3D.h"

namespace Deak {

    void Renderer::Init()
    {
        DK_PROFILE_FUNC();

        RenderCommand::Init();
        Renderer2D::Init();
        // Renderer3D::Init();
    }

    void Renderer::Shutdown()
    {
        DK_PROFILE_FUNC();

        // Renderer3D::Shutdown();
        Renderer2D::Shutdown();
    }

    void Renderer::OnWindowResize(uint32_t width, uint32_t height)
    {
        RenderCommand::SetViewport(0, 0, width, height);
    }

}

