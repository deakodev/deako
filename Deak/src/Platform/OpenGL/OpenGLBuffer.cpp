#include "OpenGLBuffer.h"
#include "dkpch.h"

#include <glad/glad.h>

namespace Deak {

    ///////////////////////////////////////////////
    ///// VertexBuffer ////////////////////////////
    ///////////////////////////////////////////////

    OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size)
    {
        DK_PROFILE_FUNC();

        glGenBuffers(1, &m_RendererID);
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    }

    OpenGLVertexBuffer::~OpenGLVertexBuffer()
    {
        DK_PROFILE_FUNC();

        glDeleteBuffers(1, &m_RendererID);
    }

    void OpenGLVertexBuffer::Bind() const
    {
        DK_PROFILE_FUNC();

        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    }

    void OpenGLVertexBuffer::Unbind() const
    {
        DK_PROFILE_FUNC();

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }


    ///////////////////////////////////////////////
    ///// IndexBuffer /////////////////////////////
    ///////////////////////////////////////////////

    OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count)
        : m_Count(count)
    {
        DK_PROFILE_FUNC();

        glGenBuffers(1, &m_RendererID);
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
    }

    OpenGLIndexBuffer::~OpenGLIndexBuffer()
    {
        DK_PROFILE_FUNC();

        glDeleteBuffers(1, &m_RendererID);
    }

    void OpenGLIndexBuffer::Bind() const
    {
        DK_PROFILE_FUNC();

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
    }

    void OpenGLIndexBuffer::Unbind() const
    {
        DK_PROFILE_FUNC();

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

}
