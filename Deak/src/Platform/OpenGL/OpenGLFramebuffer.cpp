#include "OpenGLFramebuffer.h"
#include "dkpch.h"

#include <glad/glad.h>

namespace Deak {

    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpec& spec)
        : m_Specification(spec)
    {
        Invalidate();
    }

    OpenGLFramebuffer::~OpenGLFramebuffer()
    {
        glDeleteFramebuffers(1, &m_RendererID);
        glDeleteTextures(1, &m_ColorAttachment);
        glDeleteRenderbuffers(1, &m_DepthAttachment);
    }

    void OpenGLFramebuffer::Invalidate()
    {
        if (m_RendererID)
        {
            glDeleteFramebuffers(1, &m_RendererID);
            glDeleteTextures(1, &m_ColorAttachment);
            glDeleteRenderbuffers(1, &m_DepthAttachment);
        }

        glGenFramebuffers(1, &m_RendererID);
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

        // Color attachment
        glGenTextures(1, &m_ColorAttachment);
        glBindTexture(GL_TEXTURE_2D, m_ColorAttachment);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Specification.width, m_Specification.height,
            0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glBindTexture(GL_TEXTURE_2D, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0);

        // Depth attachment
        glGenRenderbuffers(1, &m_DepthAttachment);
        glBindRenderbuffer(GL_RENDERBUFFER, m_DepthAttachment);

        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Specification.width, m_Specification.height);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER, m_DepthAttachment);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        DK_CORE_ASSERT(status == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLFramebuffer::Bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
          glViewport(0, 0, m_Specification.width, m_Specification.height);
    }

    void OpenGLFramebuffer::Unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
    {
        m_Specification.width = width;
        m_Specification.height = height;
        Invalidate();
    }

}
