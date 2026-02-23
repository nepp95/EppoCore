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

        template<typename T>
        auto WriteObject(const T& value) -> bool
        {
            const bool success = T::Serialize(this, value);
            EP_ASSERT(success);
            return success;
        }

        template<std::ranges::range T>
        auto WriteRange(const T& range) -> void
        {
            // Write range size
            WriteRaw<uint32_t>(static_cast<uint32_t>(std::ranges::size(range)));

            using ValueType = std::ranges::range_value_t<T>;

            // Write data
            for (const auto& value : range)
            {
                if constexpr (std::is_trivial<ValueType>())
                    WriteRaw<T>(value);
                else
                    WriteObject<T>(value);
            }
        }

        template<typename Key, typename Value>
        auto WriteMap(const std::map<Key, Value>& map) -> void
        {
            // Write map size
            WriteRaw<uint32_t>(static_cast<uint32_t>(map.size()));

            // Write key/values
            for (const auto& [key, value] : map)
            {
                if constexpr (std::is_trivial<Key>())
                    WriteRaw<Key>(key);
                else
                    WriteObject<Key>(key);

                if constexpr (std::is_trivial<Value>())
                    WriteRaw<Value>(value);
                else if constexpr (std::ranges::range<Value> && !std::is_same_v<Value, std::string>)
                    WriteRange<Value>(value);
                else
                    WriteObject<Value>(value);
            }
        }

        template<typename Key, typename Value>
        auto WriteMap(const std::unordered_map<Key, Value>& map) -> void
        {
            // Write map size
            WriteRaw<uint32_t>(static_cast<uint32_t>(map.size()));

            // Write key/values
            for (const auto& [key, value] : map)
            {
                if constexpr (std::is_trivial<Key>())
                    WriteRaw<Key>(key);
                else
                    WriteObject<Key>(key);

                if constexpr (std::is_trivial<Value>())
                    WriteRaw<Value>(value);
                else if constexpr (std::ranges::range<Value> && !std::is_same_v<Value, std::string>)
                    WriteRange<Value>(value);
                else
                    WriteObject<Value>(value);
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

                if constexpr (std::is_trivial<Value>())
                    WriteRaw<Value>(value);
                else
                    WriteObject<Value>(value);
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
