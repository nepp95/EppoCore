#pragma once

#include "EppoCore/Core/Buffer.h"

namespace Eppo
{
    class BufferWriter
    {
    public:
        explicit BufferWriter(uint64_t size = 1024);
        explicit BufferWriter(Buffer buffer, uint64_t position = 0);
        ~BufferWriter();

        auto WriteData(const char* data, size_t size) -> bool;

        template<typename T>
        auto WriteRaw(const T& value) -> bool
        {
            const bool success = WriteData(reinterpret_cast<const char*>(&value), sizeof(T));
            EP_ASSERT(success);
            return success;
        }

        auto WriteZero(uint64_t size = 0) -> void;
        auto WriteString(const std::string& str) -> void;
        auto WriteString(std::string_view str) -> void;

        [[nodiscard]] auto GetStreamPosition() const { return m_BufferPosition; }
        [[nodiscard]] auto GetBuffer() const { return Buffer(m_Buffer, m_BufferPosition); }

    private:
        Buffer m_Buffer;
        uint64_t m_BufferPosition = 0;
        bool m_OwningBuffer = false;
    };
}
