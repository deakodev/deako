#pragma once

#include "RendererAPI.h" 
#include "RenderCommand.h"   

namespace Deak {

    class Renderer
    {
    public:
        static void Init();
        static void Shutdown();
        static void OnWindowResize(uint32_t width, uint32_t height);
        static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
    };

}
