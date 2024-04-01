#pragma once

#include "RendererAPI.h"

namespace Deak {

    class RenderCommand
    {
    public:
        inline static void SetClearColor(const glm::vec4& color) { s_RendererAPI->SetClearColor(color); }
        inline static void Clear() { s_RendererAPI->Clear(); }

        inline static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) { s_RendererAPI->DrawIndexed(vertexArray); }
        inline static void DrawArrays(const std::shared_ptr<VertexArray>& vertexArray) { s_RendererAPI->DrawArrays(vertexArray); }

    private:
        static RendererAPI* s_RendererAPI;

    };

}
