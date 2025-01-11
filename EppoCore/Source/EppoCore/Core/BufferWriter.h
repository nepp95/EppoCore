#pragma once

#include "EppoCore/Core/Base.h"
#include "EppoCore/Core/Buffer.h"

namespace Eppo
{
	class BufferWriter
	{
	public:
		BufferWriter(Buffer buffer, uint32_t offset = 0)
			: m_Buffer(buffer), m_Offset(offset)
		{}

		void Write(Buffer srcBuffer)
		{
			EPPO_ASSERT(m_Buffer.Size >= srcBuffer.Size + m_Offset)
			memcpy(m_Buffer.Data, srcBuffer.Data, srcBuffer.Size);
			m_Offset += srcBuffer.Size;
		}
		
		void Write(void* data, uint32_t dataSize)
		{
			EPPO_ASSERT(m_Buffer.Size >= m_Offset + dataSize)
			memcpy(m_Buffer.Data, data, dataSize);
			m_Offset += dataSize;
		}

		uint32_t BytesWritten() const
		{
			return m_Offset;
		}

	private:
		Buffer m_Buffer;
		uint32_t m_Offset;
	};
}