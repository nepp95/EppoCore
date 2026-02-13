#pragma once

#include "EppoCore/Core/Buffer.h"

namespace Eppo
{
    class BufferWriter
    {
    public:
        BufferWriter(Buffer buffer, const uint64_t offset = 0)
            : m_Buffer(buffer), m_Offset(offset)
        {
        }

        void Write(Buffer srcBuffer)
        {
            EP_ASSERT(m_Buffer.Size >= srcBuffer.Size + m_Offset);
            memcpy(m_Buffer.Data, srcBuffer.Data, srcBuffer.Size);
            m_Offset += srcBuffer.Size;
        }

        void Write(void* data, const uint64_t dataSize)
        {
            EP_ASSERT(m_Buffer.Size >= m_Offset + dataSize);
            memcpy(m_Buffer.Data, data, dataSize);
            m_Offset += dataSize;
        }

        uint64_t BytesWritten() const { return m_Offset; }

    private:
        Buffer m_Buffer;
        uint64_t m_Offset;
    };
}
