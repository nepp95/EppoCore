#include "pch.h"
#include "Query.h"

#include <glad/glad.h>

namespace Eppo
{
	Query::Query()
	{
		glCreateQueries(GL_TIME_ELAPSED, 1, &m_RendererID);
	}

	Query::~Query()
	{
		glDeleteQueries(1, &m_RendererID);
	}

	void Query::Begin() const
	{
		glBeginQuery(GL_TIME_ELAPSED, m_RendererID);
	}

	void Query::End() const
	{
		glEndQuery(GL_TIME_ELAPSED);
	}

	uint64_t Query::GetResults() const
	{
		GLint64 queryResult;
		glGetQueryObjecti64v(m_RendererID, GL_QUERY_RESULT, &queryResult);

		return queryResult;
	}
}
