#include "OpenGLShader.h"
#include "dkpch.h"

#include <fstream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Deak {

    static GLenum ShaderTypeFromString(std::string_view type)
    {
        if (type == "vertex") return GL_VERTEX_SHADER;
        if (type == "fragment" || type == "pixel") return GL_FRAGMENT_SHADER;

        DK_CORE_ASSERT(false, "Unknown shader type!");
        return 0;
    }

    OpenGLShader::OpenGLShader(std::string_view filePath)
    {
        std::string source = ReadFile(filePath);
        auto shaderSources = PreProcess(source);
        Compile(shaderSources);

        auto lastSlash = filePath.find_last_of("/\\");
        lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
        auto lastDecimal = filePath.rfind(".");
        auto count = lastDecimal == std::string::npos ? filePath.size() - lastSlash : lastDecimal - lastSlash;
        m_Name = filePath.substr(lastSlash, count);
    }

    OpenGLShader::OpenGLShader(std::string_view name, const std::string& vertexSource, const std::string& fragmentSource)
        : m_Name(name)
    {
        std::unordered_map<GLenum, std::string> sources;
        sources[GL_VERTEX_SHADER] = vertexSource;
        sources[GL_FRAGMENT_SHADER] = fragmentSource;
        Compile(sources);
    }

    OpenGLShader::~OpenGLShader()
    {
        glDeleteProgram(m_RendererID);
    }

    std::string OpenGLShader::ReadFile(std::string_view filePath)
    {
        std::string result;
        std::ifstream in(filePath, std::ios::in | std::ios::binary);
        if (in)
        {
            in.seekg(0, std::ios::end);
            size_t size = in.tellg();
            if (size != -1)
            {
                result.resize(size);
                in.seekg(0, std::ios::beg);
                in.read(&result[0], size);
                in.close();
            }
            else
            {
                DK_CORE_ERROR("Could not read from file '{0}'", filePath);
            }
        }
        else
        {
            DK_CORE_ERROR("Could not open file '{0}'", filePath);
        }

        return result;
    }

    std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
    {
        std::unordered_map<GLenum, std::string> shaderSources;

        const char* typeToken = "#type";
        size_t typeTokenLength = strlen(typeToken);
        size_t pos = source.find(typeToken, 0);
        while (pos != std::string::npos)
        {
            size_t eol = source.find_first_of("\r\n", pos);
            DK_CORE_ASSERT(eol != std::string::npos, "Syntax error");
            size_t begin = pos + typeTokenLength + 1;
            std::string type = source.substr(begin, eol - begin);
            DK_CORE_ASSERT(ShaderTypeFromString(type), "Invalid shader type specified!");

            size_t nextLinePos = source.find_first_not_of("\r\n", eol);
            DK_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error");
            pos = source.find(typeToken, nextLinePos);
            shaderSources[ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
        }

        return shaderSources;
    }

    void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
    {
        GLuint program = glCreateProgram();
        DK_CORE_ASSERT(shaderSources.size() <= 2, "Only supports 2 shaders for now.");
        std::array<GLuint, 2> glShaderIDs;
        int glShaderIDIndex = 0;

        for (auto& kv : shaderSources)
        {
            GLenum type = kv.first;
            const std::string& source = kv.second;

            GLuint shader = glCreateShader(type);

            const GLchar* sourceCStr = source.c_str();
            glShaderSource(shader, 1, &sourceCStr, 0);

            glCompileShader(shader);

            GLint isCompiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
            if (isCompiled == GL_FALSE)
            {
                GLint maxLength = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

                std::vector<GLchar> infoLog(maxLength);
                glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

                glDeleteShader(shader);

                DK_CORE_ERROR("{0}", infoLog.data());
                DK_CORE_ASSERT(false, "Shader compilation failure!");
                break;
            }

            glAttachShader(program, shader);
            glShaderIDs[glShaderIDIndex++] = shader;
        }

        glLinkProgram(program);

        GLint isLinked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
        if (isLinked == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

            glDeleteProgram(program);

            for (auto id : glShaderIDs)
                glDeleteShader(id);

            DK_CORE_ERROR("{0}", infoLog.data());
            DK_CORE_ASSERT(false, "Shader link failure!");
            return;
        }

        for (auto id : glShaderIDs)
        {
            glDetachShader(program, id);
            glDeleteShader(id);
        }

        m_RendererID = program;
    }

    void OpenGLShader::Bind() const
    {
        glUseProgram(m_RendererID);
    }

    void OpenGLShader::Unbind() const
    {
        glUseProgram(0);
    }

    void OpenGLShader::SetInt(const std::string& name, int value)
    {
        UploadUniformInt(name, value);
    }

    void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
    {
        UploadUniformVec3(name, value);
    }

    void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
    {
        UploadUniformVec4(name, value);
    }

    void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
    {
        UploadUniformMat4(name, value);
    }

    int OpenGLShader::GetUniformLocation(const std::string& name) const
    {
        if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
            return m_UniformLocationCache[name];

        int location = glGetUniformLocation(m_RendererID, name.c_str());
        if (location == -1)
        {
            DK_CORE_ERROR("Uniform {0} not found!", name);
            return location;
        }

        m_UniformLocationCache[name] = location;

        return location;
    }

    void OpenGLShader::UploadUniformBool(const std::string& name, bool value) const
    {
        glUniform1i(GetUniformLocation(name), (int)value);
    }

    void OpenGLShader::UploadUniformInt(const std::string& name, int value) const
    {
        glUniform1i(GetUniformLocation(name), value);
    }

    void OpenGLShader::UploadUniformFloat(const std::string& name, float value) const
    {
        glUniform1f(GetUniformLocation(name), value);
    }

    void OpenGLShader::UploadUniformVec2(const std::string& name, const glm::vec2& value) const
    {
        glUniform2fv(GetUniformLocation(name), 1, &value[0]);
    }

    void OpenGLShader::UploadUniformVec2(const std::string& name, float x, float y) const
    {
        glUniform2f(GetUniformLocation(name), x, y);
    }

    void OpenGLShader::UploadUniformVec3(const std::string& name, const glm::vec3& value) const
    {
        glUniform3fv(GetUniformLocation(name), 1, &value[0]);
    }

    void OpenGLShader::UploadUniformVec3(const std::string& name, float x, float y, float z) const
    {
        glUniform3f(GetUniformLocation(name), x, y, z);
    }

    void OpenGLShader::UploadUniformVec4(const std::string& name, const glm::vec4& value) const
    {
        glUniform4fv(GetUniformLocation(name), 1, &value[0]);
    }

    void OpenGLShader::UploadUniformVec4(const std::string& name, float x, float y, float z, float w) const
    {
        glUniform4f(GetUniformLocation(name), x, y, z, w);
    }

    void OpenGLShader::UploadUniformMat2(const std::string& name, const glm::mat2& mat) const
    {
        glUniformMatrix2fv(GetUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
    }

    void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& mat) const
    {
        glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
    }

    void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& mat) const
    {
        glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
    }

}
