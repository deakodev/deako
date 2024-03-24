#include "VertexArray.h"
#include "dkpch.h"

#include "Renderer.h"

#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Deak {

    VertexArray* VertexArray::Create()
    {
        switch (Renderer::GetAPI())
        {
        case RendererAPI::None:
            DK_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
            return nullptr;
        case RendererAPI::OpenGL:
            return new OpenGLVertexArray();
        }
        DK_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

}
