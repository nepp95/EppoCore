#pragma once

#include "EppoCore/Core/Buffer.h"

namespace Eppo
{
    class BufferReader
    {
    public:
        explicit BufferReader(Buffer buffer, uint64_t position = 0);

        auto ReadData(char* outData, size_t size) -> bool;
        auto ReadString(std::string& str) -> bool;

        template<typename T>
        auto ReadRaw(T& value) -> bool
        {
            const bool success = ReadData(reinterpret_cast<char*>(&value), sizeof(T));
            EP_ASSERT(success);
            return success;
        }

        template<typename T>
        auto ReadObject(T& value) -> bool
        {
            const bool success = T::Deserialize(this, value);
            EP_ASSERT(success);
            return success;
        }

        template<std::ranges::range T>
        auto ReadRange(T& range) -> void
        {
            // Read range size
            uint32_t size;
            ReadRaw<uint32_t>(size);

            // Range must be resizable
            if constexpr (requires(T t, size_t n) { t.resize(n); })
            {
                range.resize(size);

                using ValueType = std::ranges::range_value_t<T>;

                // Read data
                for (auto& element : range)
                {
                    if constexpr (std::is_trivial<ValueType>())
                        ReadRaw<ValueType>(element);
                    else
                        ReadObject<ValueType>(element);
                }
            }
            else
            {
                static_assert(false);
            }
        }

        template<typename Key, typename Value>
        auto ReadMap(std::map<Key, Value>& map) -> void
        {
            // Read map size
            uint32_t size;
            ReadRaw<uint32_t>(size);

            // Read key/values
            for (uint32_t i = 0; i < size; i++)
            {
                Key key;

                if constexpr (std::is_trivial<Key>())
                    ReadRaw<Key>(key);
                else
                    ReadObject<Key>(key);

                if constexpr (std::is_trivial<Value>())
                    ReadRaw<Value>(map[key]);
                else if constexpr (std::ranges::range<Value> && !std::is_same_v<Value, std::string>)
                    ReadRange<Value>(map[key]);
                else
                    ReadObject<Value>(map[key]);
            }
        }

        template<typename Key, typename Value>
        auto ReadMap(std::unordered_map<Key, Value>& map) -> void
        {
            // Read map size
            uint32_t size;
            ReadRaw<uint32_t>(size);

            // Read key/values
            for (uint32_t i = 0; i < size; i++)
            {
                Key key;

                if constexpr (std::is_trivial<Key>())
                    ReadRaw<Key>(key);
                else
                    ReadObject<Key>(key);

                if constexpr (std::is_trivial<Value>())
                    ReadRaw<Value>(map[key]);
                else if constexpr (std::ranges::range<Value> && !std::is_same_v<Value, std::string>)
                    ReadRange<Value>(map[key]);
                else
                    ReadObject<Value>(map[key]);
            }
        }

        template<typename Value>
        auto ReadMap(std::unordered_map<std::string, Value>& map) -> void
        {
            // Read map size
            uint32_t size;
            ReadRaw<uint32_t>(size);

            // Read key/values
            for (uint32_t i = 0; i < size; i++)
            {
                std::string key;
                ReadString(key);

                if constexpr (std::is_trivial<Value>())
                    ReadRaw<Value>(map[key]);
                else
                    ReadObject<Value>(map[key]);
            }
        }

        [[nodiscard]] auto GetStreamPosition() const { return m_BufferPosition; }
        [[nodiscard]] auto GetBuffer() const { return Buffer(m_Buffer, m_BufferPosition); }

    private:
        Buffer m_Buffer;
        uint64_t m_BufferPosition = 0;
    };
}
