#pragma once

#include "EppoCore/Core/Base.h"
#include "EppoCore/Core/Buffer.h"

namespace Eppo
{
	class BufferReader
	{
	public:
		BufferReader(Buffer buffer)
			: m_Buffer(buffer)
		{}

		template<typename T>
		T Read()
		{
			EPPO_ASSERT(m_Buffer.Size >= m_Offset + sizeof(T))
			m_Offset += sizeof(T);
			return static_cast<T>(m_Buffer.Data + m_Offset);
		}

	private:
		Buffer m_Buffer;
		uint32_t m_Offset = 0;
	};
}