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
        auto WriteZero(uint64_t size = 0) -> void;
        auto WriteString(const std::string& str) -> void;
        auto WriteString(std::string_view str) -> void;

        template<typename T>
        auto WriteRaw(const T& value) -> bool
        {
            const bool success = WriteData(reinterpret_cast<const char*>(&value), sizeof(T));
            EP_ASSERT(success);
            return success;
        }

        template<typename Key, typename Value>
            requires(std::is_trivial<Key>())
        auto WriteMap(const std::map<Key, Value>& map) -> void
        {
            // Write map size
            WriteRaw<uint32_t>(static_cast<uint32_t>(map.size()));

            // Write key/values
            for (const auto& [key, value] : map)
            {
                WriteRaw<Key>(key);
                WriteRaw<Value>(value);
            }
        }

        template<typename Key, typename Value>
            requires(std::is_trivial<Key>())
        auto WriteMap(const std::unordered_map<Key, Value>& map) -> void
        {
            // Write map size
            WriteRaw<uint32_t>(static_cast<uint32_t>(map.size()));

            // Write key/values
            for (const auto& [key, value] : map)
            {
                WriteRaw<Key>(key);
                WriteRaw<Value>(value);
            }
        }

        template<typename Value>
        auto WriteMap(const std::unordered_map<std::string, Value>& map) -> void
        {
            // Write map size
            WriteRaw<uint32_t>(static_cast<uint32_t>(map.size()));

            // Write key/values
            for (const auto& [key, value] : map)
            {
                WriteString(key);
                WriteRaw<Value>(value);
            }
        }

        [[nodiscard]] auto GetStreamPosition() const { return m_BufferPosition; }
        [[nodiscard]] auto GetBuffer() const { return Buffer(m_Buffer, m_BufferPosition); }

    private:
        Buffer m_Buffer;
        uint64_t m_BufferPosition = 0;
        bool m_OwningBuffer = false;
    };
}
