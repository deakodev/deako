#pragma once 

#include "Deak/Renderer/Framebuffer.h"

namespace Deak {

    class OpenGLFramebuffer : public Framebuffer
    {
    public:
        OpenGLFramebuffer(const FramebufferSpec& spec);
        virtual ~OpenGLFramebuffer();

        void Invalidate();

        virtual void Bind() override;
        virtual void Unbind() override;

        virtual uint32_t GetColorAttachmentRendererID() const override { return m_ColorAttachment; }

        virtual const FramebufferSpec& GetSpecification() const override { return m_Specification; }

    private:
        uint32_t m_RendererID;
        uint32_t m_ColorAttachment;
        uint32_t m_DepthAttachment;
        FramebufferSpec m_Specification;

    };

}
