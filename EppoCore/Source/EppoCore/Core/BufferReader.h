#pragma once

#include "EppoCore/Core/Buffer.h"

namespace Eppo
{
    class BufferReader
    {
    public:
        explicit BufferReader(Buffer buffer, uint64_t position = 0);

        auto ReadData(char* outData, size_t size) -> bool;

        template<typename T>
        auto ReadRaw(T& value) -> bool
        {
            const bool success = ReadData(reinterpret_cast<char*>(&value), sizeof(T));
            EP_ASSERT(success);
            return success;
        }

        auto ReadString(std::string& str) -> bool;

        [[nodiscard]] auto GetStreamPosition() const { return m_BufferPosition; }
        [[nodiscard]] auto GetBuffer() const { return Buffer(m_Buffer, m_BufferPosition); }

    private:
        Buffer m_Buffer;
        uint64_t m_BufferPosition = 0;
    };
}
