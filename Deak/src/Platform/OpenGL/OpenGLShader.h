#pragma once

#include "Deak/Renderer/Shader.h"
#include <glm/glm.hpp>

namespace Deak {

    class OpenGLShader : public Shader
    {
    public:
        OpenGLShader(const std::string& vertexSource, const std::string& fragmentSource);
        virtual ~OpenGLShader();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        void setUniformBool(const std::string& name, bool value) const;
        void setUniformInt(const std::string& name, int value) const;
        void setUniformFloat(const std::string& name, float value) const;
        void setUniformVec2(const std::string& name, const glm::vec2& value) const;
        void setUniformVec2(const std::string& name, float x, float y) const;
        void setUniformVec3(const std::string& name, const glm::vec3& value) const;
        void setUniformVec3(const std::string& name, float x, float y, float z) const;
        void setUniformVec4(const std::string& name, const glm::vec4& value) const;
        void setUniformVec4(const std::string& name, float x, float y, float z, float w) const;
        void setUniformMat2(const std::string& name, const glm::mat2& mat) const;
        void setUniformMat3(const std::string& name, const glm::mat3& mat) const;
        void setUniformMat4(const std::string& name, const glm::mat4& mat) const;

    private:
        int GetUniformLocation(const std::string& name) const;

    private:
        uint32_t m_RendererID;
        mutable std::unordered_map<std::string, int> m_UniformLocationCache;
    };

}
