#pragma once

#include "EppoCore/Core/Buffer.h"

namespace Eppo
{
    class BufferWriter
    {
    public:
        explicit BufferWriter(uint64_t size = 1024);
        explicit BufferWriter(Buffer buffer, uint64_t position = 0);

        auto WriteData(const char* data, const size_t size) -> bool;

        template<typename T>
        auto WriteRaw(const T& value) -> bool;

        auto WriteZero(uint64_t size = 0) -> void;
        auto WriteString(const std::string& str) -> void;
        auto WriteString(std::string_view str) -> void;

        auto GetStreamPosition() const { return m_BufferPosition; }
        auto GetBuffer() const { return Buffer(m_Buffer, m_BufferPosition); }

    private:
        Buffer m_Buffer;
        uint64_t m_BufferPosition = 0;
    };
}
