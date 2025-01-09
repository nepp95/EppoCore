#pragma once

#include <glm/glm.hpp>

namespace Eppo
{
	struct Buffer
    {
        uint8_t* Data = nullptr;
        uint32_t Size = 0;

        Buffer() = default;

        Buffer(const uint32_t size)
        {
            Allocate(size);
        }

        static Buffer Copy(const Buffer other)
        {
            const Buffer result(other.Size);
            memcpy(result.Data, other.Data, other.Size);
            return result;
        }

        static Buffer Copy(const uint8_t* data, const uint32_t size)
        {
            const Buffer result(size);
            memcpy(result.Data, data, size);
            return result;
        }

        static Buffer Copy(const void* data, const uint32_t size)
        {
            const Buffer result(size);
            memcpy(result.Data, data, size);
            return result;
        }

        void Allocate(const uint32_t size)
        {
            Release();

            Data = new uint8_t[size]{};
            Size = size;
        }

        void Release()
        {
            delete[] Data;
            Data = nullptr;
            Size = 0;
        }

        template<typename T>
        T* As()
        {
            return (T*)Data;
        }

        explicit operator bool() const
        {
            return static_cast<bool>(Data);
        }
    };

	class ScopedBuffer
    {
    public:
        ScopedBuffer() = default;

        explicit ScopedBuffer(const Buffer buffer)
            : m_Buffer(buffer)
        {}

        ScopedBuffer(const uint32_t size)
            : m_Buffer(size)
        {}

        ~ScopedBuffer()
        {
            m_Buffer.Release();
        }

        [[nodiscard]] uint8_t* Data() const
        {
            return m_Buffer.Data;
        }

        [[nodiscard]] uint32_t Size() const
        {
            return m_Buffer.Size;
        }

        template<typename T>
        T* As()
        {
            return m_Buffer.As<T>();
        }

    private:
        Buffer m_Buffer;
    };
}
