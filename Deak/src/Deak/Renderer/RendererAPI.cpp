#include "RendererAPI.h"
#include "dkpch.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Deak {

    RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

    Scope<RendererAPI> RendererAPI::Create()
    {
        switch (s_API)
        {
        case RendererAPI::API::None:    DK_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
        case RendererAPI::API::OpenGL:  return CreateScope<OpenGLRendererAPI>();
        }

        DK_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

}
