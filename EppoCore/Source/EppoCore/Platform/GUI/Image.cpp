#include "pch.h"
#include "Image.h"

namespace Eppo
{
    Image::Image(const uint32_t width, const uint32_t height)
        : m_Width(width), m_Height(height)
    {
    }

    Image::~Image()
    {
    }

    void Image::SetData(const void* data) const
    {
    }

    void Image::Bind(const uint32_t slot) const
    {
    }
}
