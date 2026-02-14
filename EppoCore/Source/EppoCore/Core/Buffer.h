#pragma once

namespace Eppo
{
    struct Buffer
    {
        uint8_t* Data = nullptr;
        uint64_t Size = 0;

        Buffer() = default;

        explicit Buffer(const uint64_t size) { Allocate(size); }
        explicit Buffer(const Buffer& other, const uint64_t size)
            : Data(other.Data), Size(size)
        {
        }

        auto Allocate(const uint64_t size) -> void
        {
            Release();

            Data = new uint8_t[size]{};
            Size = size;
        }

        auto Release() -> void
        {
            delete[] Data;
            Data = nullptr;
            Size = 0;
        }

        static auto Copy(const Buffer other) -> Buffer
        {
            const Buffer result(other.Size);
            std::memcpy(result.Data, other.Data, other.Size);
            return result;
        }

        static auto Copy(const uint8_t* data, const uint64_t size) -> Buffer
        {
            const Buffer result(size);
            std::memcpy(result.Data, data, size);
            return result;
        }

        static auto Copy(const void* data, const uint64_t size) -> Buffer
        {
            const Buffer result(size);
            std::memcpy(result.Data, data, size);
            return result;
        }

        template<typename T>
        auto As() -> T*
        {
            return reinterpret_cast<T*>(Data);
        }

        template<typename T>
        auto As() const -> const T*
        {
            return reinterpret_cast<const T*>(Data);
        }

        explicit operator bool() const { return static_cast<bool>(Data); }
    };

    class ScopedBuffer
    {
    public:
        ScopedBuffer() = default;

        explicit ScopedBuffer(const Buffer buffer)
            : m_Buffer(buffer)
        {
        }

        explicit ScopedBuffer(const uint64_t size)
            : m_Buffer(size)
        {
        }

        ~ScopedBuffer() { m_Buffer.Release(); }

        [[nodiscard]] auto Data() const -> uint8_t* { return m_Buffer.Data; }
        [[nodiscard]] auto Size() const -> uint64_t { return m_Buffer.Size; }

        template<typename T>
        auto As() -> T*
        {
            return m_Buffer.As<T>();
        }

        template<typename T>
        auto As() const -> const T*
        {
            return m_Buffer.As<T>();
        }

    private:
        Buffer m_Buffer;
    };
}
