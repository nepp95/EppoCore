#include "pch.h"
#include "DeletionQueue.h"

namespace Eppo
{
    auto DeletionQueue::Push(const QueueValues& values, std::function<void()> destroy) -> void
    {
        m_Entries.push_back({ values, std::move(destroy) });
    }

    auto DeletionQueue::Collect(const QueueValues& completed) -> void
    {
        ZoneScoped;

        while (!m_Entries.empty())
        {
            const auto& values = m_Entries.front().Values;

            bool done = true;
            for (size_t i = 0; i < QueueTypeCount; i++)
                done &= values[i] <= completed[i];

            // Entries are pushed in submission order, so the first unfinished one ends the scan
            if (!done)
                break;

            // Destroying may push new entries, so take it out of the queue first
            auto destroy = std::move(m_Entries.front().Destroy);
            m_Entries.pop_front();
            destroy();
        }
    }

    auto DeletionQueue::Flush() -> void
    {
        ZoneScoped;

        while (!m_Entries.empty())
        {
            auto destroy = std::move(m_Entries.front().Destroy);
            m_Entries.pop_front();
            destroy();
        }
    }
}
