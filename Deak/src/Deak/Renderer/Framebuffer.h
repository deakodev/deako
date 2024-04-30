#pragma once

#include "Deak/Core/Base.h"

namespace Deak {

    struct FramebufferSpec
    {
        uint32_t width;
        uint32_t height;
        uint32_t samples = 1;

        bool swapChainTarget = false;
    };

    class Framebuffer
    {
    public:
        virtual ~Framebuffer() = default;

        virtual void Bind() = 0;
        virtual void Unbind() = 0;

        virtual uint32_t GetColorAttachmentRendererID() const = 0;
        virtual const FramebufferSpec& GetSpecification() const = 0;

        virtual void Resize(uint32_t width, uint32_t height) = 0;

        static Ref<Framebuffer> Create(const FramebufferSpec& spec);

    };

}
