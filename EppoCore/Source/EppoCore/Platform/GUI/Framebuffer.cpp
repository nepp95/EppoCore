#include "pch.h"
#include "Framebuffer.h"

namespace Eppo
{
    Framebuffer::Framebuffer(const FramebufferSpecification spec)
        : m_Specification(spec)
    {
    }

    Framebuffer::~Framebuffer()
    {
    }

    void Framebuffer::Bind() const
    {
    }

    void Framebuffer::Unbind()
    {
    }
}
