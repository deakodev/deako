#include "Shader.h"
#include "dkpch.h"

#include "Renderer.h"

#include "Platform/OpenGL/OpenGLShader.h"

namespace Deak {

    Shader* Shader::Create(const std::string& vertexSource, const std::string& fragmentSource)
    {
        switch (Renderer::GetAPI())
        {
        case RendererAPI::API::None:
            DK_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
            return nullptr;
        case RendererAPI::API::OpenGL:
            return new OpenGLShader(vertexSource, fragmentSource);
        }
        DK_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

}
