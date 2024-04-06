#pragma once

#include "Deak/Renderer/Shader.h"
#include <glm/glm.hpp>

// TODO: should remove later
typedef unsigned int GLenum;

namespace Deak {

    class OpenGLShader : public Shader
    {
    public:
        OpenGLShader(const std::string& filePath);
        OpenGLShader(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource);
        virtual ~OpenGLShader();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void SetInt(const std::string& name, int value) override;
        virtual void SetFloat3(const std::string& name, const glm::vec3& value) override;
        virtual void SetFloat4(const std::string& name, const glm::vec4& value) override;
        virtual void SetMat4(const std::string& name, const glm::mat4& value) override;

        virtual const std::string& GetName() const override { return m_Name; }

        void UploadUniformBool(const std::string& name, bool value) const;
        void UploadUniformInt(const std::string& name, int value) const;
        void UploadUniformFloat(const std::string& name, float value) const;
        void UploadUniformVec2(const std::string& name, const glm::vec2& value) const;
        void UploadUniformVec2(const std::string& name, float x, float y) const;
        void UploadUniformVec3(const std::string& name, const glm::vec3& value) const;
        void UploadUniformVec3(const std::string& name, float x, float y, float z) const;
        void UploadUniformVec4(const std::string& name, const glm::vec4& value) const;
        void UploadUniformVec4(const std::string& name, float x, float y, float z, float w) const;
        void UploadUniformMat2(const std::string& name, const glm::mat2& mat) const;
        void UploadUniformMat3(const std::string& name, const glm::mat3& mat) const;
        void UploadUniformMat4(const std::string& name, const glm::mat4& mat) const;

    private:
        std::string ReadFile(const std::string& filePath);
        std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
        void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);
        int GetUniformLocation(const std::string& name) const;

    private:
        uint32_t m_RendererID;
        std::string m_Name;
        mutable std::unordered_map<std::string, int> m_UniformLocationCache;
    };

}
