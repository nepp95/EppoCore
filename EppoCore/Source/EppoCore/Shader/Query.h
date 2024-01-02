#pragma once

namespace Eppo
{
	class Query
	{
	public:
		Query();
		~Query();

		void Begin() const;
		void End() const;

		uint64_t GetResults() const;

	private:
		uint32_t m_RendererID;
	};
}
