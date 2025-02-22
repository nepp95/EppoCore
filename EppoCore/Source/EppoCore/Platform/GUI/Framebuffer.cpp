#include "Framebuffer.h"

#include <glad/glad.h>

namespace Eppo
{
    Framebuffer::Framebuffer(const FramebufferSpecification spec)
        : m_Specification(spec)
    {
        glCreateFramebuffers(1, &m_FramebufferID);
        glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);

        glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID);
        glBindTexture(GL_TEXTURE_2D, m_TextureID);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<int>(m_Specification.Width), static_cast<int>(m_Specification.Height), 0,
                     GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureID, 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);

        EPPO_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    Framebuffer::~Framebuffer()
    {
        glDeleteTextures(1, &m_TextureID);
        glDeleteFramebuffers(1, &m_FramebufferID);
    }

    void Framebuffer::Bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);
        glViewport(0, 0, static_cast<int>(m_Specification.Width), static_cast<int>(m_Specification.Height));
    }

    void Framebuffer::Unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}
