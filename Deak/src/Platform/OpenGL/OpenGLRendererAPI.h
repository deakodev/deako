#pragma once

#include "Deak/Renderer/RendererAPI.h"

namespace Deak {

    class OpenGLRendererAPI : public RendererAPI
    {
    public:
        virtual void SetClearColor(const glm::vec4& color) override;
        virtual void Clear() override;

        virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) override;
        virtual void DrawArrays(const Ref<VertexArray>& vertexArray) override;

    private:

    };

}
