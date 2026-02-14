#include "pch.h"
#include "BufferReader.h"

namespace Eppo
{
    BufferReader::BufferReader(const Buffer buffer, const uint64_t position)
        : m_Buffer(buffer), m_BufferPosition(position)
    {
    }

    auto BufferReader::ReadData(char* outData, const size_t size) -> bool
    {
        const bool valid = m_BufferPosition + size <= m_Buffer.Size;
        EP_ASSERT(valid);
        if (!valid)
            return false;

        std::memcpy(outData, m_Buffer.As<uint8_t>() + m_BufferPosition, size);
        m_BufferPosition += size;

        return true;
    }

    template<typename T>
    auto BufferReader::ReadRaw(T& value) -> bool
    {
        const bool success = ReadData(reinterpret_cast<char*>(&value), sizeof(T));
        EP_ASSERT(success);
        return success;
    }

    auto BufferReader::ReadString(std::string& str) -> bool
    {
        uint64_t size = 0;
        if (!ReadData(reinterpret_cast<char*>(&size), sizeof(uint64_t)))
            return false;

        str.resize(size);
        return ReadData(str.data(), sizeof(char) * size);
    }
}
