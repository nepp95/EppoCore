#include "pch.h"
#include "BufferWriter.h"

namespace Eppo
{
    BufferWriter::BufferWriter(const uint64_t size)
        : m_OwningBuffer(true)
    {
        m_Buffer = Buffer(size);
    }

    BufferWriter::BufferWriter(const Buffer buffer, const uint64_t position)
        : m_Buffer(buffer), m_BufferPosition(position)
    {
    }

    BufferWriter::~BufferWriter()
    {
        if (m_OwningBuffer)
            m_Buffer.Release();
    }

    auto BufferWriter::WriteData(const char* data, const size_t size) -> bool
    {
        const bool valid = m_BufferPosition + size <= m_Buffer.Size;
        EP_ASSERT(valid);
        if (!valid)
            return false;

        std::memcpy(m_Buffer.As<uint8_t>() + m_BufferPosition, data, size);
        m_BufferPosition += size;

        return true;
    }

    auto BufferWriter::WriteZero(const uint64_t size) -> void
    {
        constexpr char zero = 0;

        for (uint64_t i = 0; i < size; i++)
            WriteData(&zero, 1);
    }

    auto BufferWriter::WriteString(const std::string& str) -> void
    {
        uint64_t size = str.size();
        WriteData(reinterpret_cast<char*>(&size), sizeof(uint64_t));
        WriteData(str.data(), sizeof(char) * size);
    }

    auto BufferWriter::WriteString(const std::string_view str) -> void
    {
        uint64_t size = str.size();
        WriteData(reinterpret_cast<char*>(&size), sizeof(uint64_t));
        WriteData(str.data(), sizeof(char) * size);
    }
}
