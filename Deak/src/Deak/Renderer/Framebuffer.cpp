#include "Framebuffer.h"
#include "dkpch.h"

#include "Renderer.h"

#include "Platform/OpenGL/OpenGLFramebuffer.h"

namespace Deak {

    Ref<Framebuffer> Framebuffer::Create(const FramebufferSpec& spec)
    {
        switch (Renderer::GetAPI())
        {
        case RendererAPI::API::None:
            DK_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
            return nullptr;
        case RendererAPI::API::OpenGL:
            return CreateRef<OpenGLFramebuffer>(spec);
        }
        DK_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}
