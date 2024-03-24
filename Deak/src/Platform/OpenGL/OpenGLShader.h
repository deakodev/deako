#pragma once

#include "dkpch.h"

#include "Deak/Renderer/Shader.h"

namespace Deak {

    class OpenGLShader : public Shader
    {
    public:
        OpenGLShader(const std::string& vertexSource, const std::string& fragmentSource);
        virtual ~OpenGLShader();

        virtual void Bind() const override;
        virtual void Unbind() const override;

    private:
        uint32_t m_RendererID;
    };

}
