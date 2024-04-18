#include "UniformBuffer.h"
#include "dkpch.h"

#include "Deak/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLUniformBuffer.h"

namespace Deak {

    Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
    {
        switch (Renderer::GetAPI())
        {
        case RendererAPI::API::None:    DK_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
        case RendererAPI::API::OpenGL:  return CreateRef<OpenGLUniformBuffer>(size, binding);
        }

        DK_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

}
