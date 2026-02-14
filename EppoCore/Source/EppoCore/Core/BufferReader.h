#pragma once

#include "EppoCore/Core/Buffer.h"

namespace Eppo
{
    class BufferReader
    {
    public:
        explicit BufferReader(Buffer buffer, uint64_t position = 0);

        auto ReadData(char* outData, const size_t size) -> bool;

        template<typename T>
        auto ReadRaw(T& value) -> bool;

        auto ReadString(std::string& str) -> bool;

        auto GetStreamPosition() const { return m_BufferPosition; }
        auto GetBuffer() const { return Buffer(m_Buffer, m_BufferPosition); }

    private:
        Buffer m_Buffer;
        uint64_t m_BufferPosition = 0;
    };
}
