#pragma once

#include "EppoCore/Core/Buffer.h"

namespace Eppo
{
    inline auto ReadBytes(const std::filesystem::path& path) -> Buffer
    {
        std::ifstream in(path, std::ios::binary);
        if (!in)
            return {};

        const auto fileSize = std::filesystem::file_size(path);
        if (fileSize == 0)
            return {};

        Buffer buffer(fileSize);
        in.read(buffer.As<char>(), static_cast<std::streamsize>(fileSize));

        return buffer;
    }

    inline auto ReadText(const std::filesystem::path& path) -> std::string
    {
        std::ifstream in(path, std::ios::binary);
        if (!in)
            return {};

        const auto fileSize = std::filesystem::file_size(path);
        if (fileSize == 0)
            return {};

        std::string text(fileSize, '\0');
        in.read(text.data(), static_cast<std::streamsize>(fileSize));

        const auto readCount = in.gcount();
        text.resize(readCount > 0 ? static_cast<size_t>(readCount) : 0);

        return text;
    }
}
