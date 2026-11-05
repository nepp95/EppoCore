#include "pch.h"
#include "Framebuffer.h"

#include "EppoCore/GPU/Device.h"

namespace Eppo
{
    Framebuffer::Framebuffer(Device& device, const FramebufferSpecification specification)
        : m_Device(&device), m_Specification(specification)
    {
        Resize(specification.Width, specification.Height);
    }

    Framebuffer::~Framebuffer() = default;

    auto Framebuffer::Resize(const uint32_t width, const uint32_t height) -> void
    {
        m_Specification.Width = width;
        m_Specification.Height = height;

        if (width == 0 || height == 0)
        {
            m_ColorAttachment.reset();
            return;
        }

        m_ColorAttachment.emplace(
            *m_Device,
            ImageSpecification{
                .Width = width,
                .Height = height,
                .Format = m_Specification.Format,
                .Usage = ImageUsage::Sampled | ImageUsage::ColorAttachment,
                .DebugName = "Framebuffer color",
            }
        );
    }
}
