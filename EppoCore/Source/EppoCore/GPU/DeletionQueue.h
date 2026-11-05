#pragma once

#include "EppoCore/GPU/Queue.h"

#include <deque>

namespace Eppo
{
    // Timeline value per queue type
    using QueueValues = std::array<uint64_t, QueueTypeCount>;

    // Destroys GPU objects only after the work that may still use them has finished
    class DeletionQueue
    {
    public:
        // Runs destroy once every queue has completed at least the given values
        auto Push(const QueueValues& values, std::function<void()> destroy) -> void;

        // Runs every destruction whose values have been reached
        auto Collect(const QueueValues& completed) -> void;

        // Runs everything; only call when the device is idle
        auto Flush() -> void;

    private:
        struct Entry
        {
            QueueValues Values{};
            std::function<void()> Destroy;
        };

        std::deque<Entry> m_Entries;
    };
}
